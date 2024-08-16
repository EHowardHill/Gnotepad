#include <ncurses.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

using namespace std;

int _argc;
char **_argv;

#define DEFAULT_FILENAME "output.txt"

class TextEditor
{
public:
    TextEditor()
    {
        initscr();
        noecho();
        keypad(stdscr, TRUE);
        getmaxyx(stdscr, max_y, max_x);

        if (_argc == 2)
        {
            ifstream inputFile(_argv[1]);
            if (inputFile.is_open())
            {
                string line;
                while (getline(inputFile, line))
                {
                    buffer.push_back(line);
                }
                inputFile.close();
            }
        }

        buffer.push_back("");
        word_wrap();

        cursor_x = 0;
        cursor_y = 0;
        view_start = 0; // Track the starting line of the view
    }

    ~TextEditor()
    {
        endwin();
    }

    void run()
    {
        while (true)
        {
            render();
            int ch = getch();
            int cont = handle_input(ch);

            if (ch == 17)
            { // Quit on Ctrl+C
                break;
            }
        }
    }

private:
    vector<string> buffer;
    int cursor_x, cursor_y;
    int max_x, max_y;
    int view_start;
    bool saved = true;

    void render()
    {
        clear();

        int word_count = 0;
        word_count++;
        for (const string &word : buffer)
        {
            word_count += count(word.begin(), word.end(), ' ');
        }

        if (_argc == 1)
        {
            mvprintw(max_y - 1, 0, "Ctrl+C to exit | output.txt | Words: %d", word_count);
        }
        else
        {
            mvprintw(max_y - 1, 0, "Ctrl+C to exit | %s | Words: %d", _argv[1], word_count);
        }

        int lines_to_display = max_y - 2;
        for (int i = 0; i < lines_to_display && view_start + i < buffer.size(); ++i)
        {
            mvprintw(i, 0, buffer[view_start + i].c_str());
        }
        move(cursor_y - view_start, cursor_x); // Adjust for view_start

        refresh();
    }

    int handle_input(int ch)
    {
        saved = true;

        switch (ch)
        {
        case KEY_UP:
            if (cursor_y > 0)
            {
                cursor_y--;
            }
            if (cursor_y < view_start)
            {
                view_start--;
            }
            if (cursor_x > buffer[cursor_y].length())
            {
                cursor_x = buffer[cursor_y].length();
            }
            break;
        case KEY_DOWN:
            if (cursor_y < buffer.size() - 1)
            {
                cursor_y++;
            }
            if (cursor_y >= view_start + max_y - 2)
            {
                view_start++;
            }
            if (cursor_x > buffer[cursor_y].length())
            {
                cursor_x = buffer[cursor_y].length();
            }
            break;
        case KEY_LEFT:
            if (cursor_x > 0)
            {
                cursor_x--;
            }
            break;
        case KEY_RIGHT:
            if (cursor_x < buffer[cursor_y].length())
            {
                cursor_x++;
            }
            break;
        case KEY_BACKSPACE:
        case 127: // Handle backspace
            if (cursor_x > 0)
            {
                buffer[cursor_y].erase(cursor_x - 1, 1);
                cursor_x--;
            }
            else if (cursor_y > 0)
            {
                cursor_x = buffer[cursor_y - 1].length();
                buffer[cursor_y - 1] += buffer[cursor_y];
                buffer.erase(buffer.begin() + cursor_y);
                cursor_y--;
                if (cursor_y < view_start)
                {
                    view_start--;
                }
            }
            break;

        case KEY_DC: // Handle Delete key
            if (cursor_x < buffer[cursor_y].length())
            {
                buffer[cursor_y].erase(cursor_x, 1);
            }
            else if (cursor_y < buffer.size() - 1)
            {
                buffer[cursor_y] += buffer[cursor_y + 1];
                buffer.erase(buffer.begin() + cursor_y + 1);
            }
            word_wrap();
            break;

        case KEY_HOME:
            cursor_x = 0;
            break;

        case KEY_END:
        {
            int cursor_x_line = cursor_x / max_x;
            int new_cursor_x = ((cursor_x_line + 1) * max_x) - 1;

            if (new_cursor_x > buffer[cursor_y].length())
            {
                new_cursor_x = buffer[cursor_y].length();
            }

            cursor_x = new_cursor_x;
        }
        break;

        case KEY_PPAGE:
            // Scroll up by a page
            view_start -= max_y - 2;
            if (view_start < 0)
            {
                view_start = 0;
            }
            break;

        case KEY_NPAGE:
            // Scroll down by a page
            view_start += max_y - 2;
            if (view_start > buffer.size() - max_y + 2)
            {
                view_start = buffer.size() - max_y + 2;
            }
            break;

        case KEY_ENTER:
        case 10: // Enter key
            buffer.insert(buffer.begin() + cursor_y + 1, buffer[cursor_y].substr(cursor_x));
            buffer[cursor_y] = buffer[cursor_y].substr(0, cursor_x);
            cursor_y++;
            cursor_x = 0;
            if (cursor_y >= view_start + max_y - 2)
            {
                view_start++;
            }
            break;
        default:
            if (isprint(ch))
            {
                buffer[cursor_y].insert(cursor_x, 1, ch);
                cursor_x++;
            }

            break;
        }

        // Save file contents
        string filename = (_argc == 2) ? _argv[1] : DEFAULT_FILENAME;
        ofstream outfile(filename);

        if (outfile.is_open())
        {
            for (const string &line : buffer)
            {
                outfile << line << endl;
            }
            outfile.close();
            saved = true;
        }

        word_wrap();

        return 1;
    }

    void word_wrap()
    {
        for (size_t i = 0; i < buffer.size(); ++i)
        {
            while (buffer[i].length() > max_x)
            {
                size_t wrap_pos = buffer[i].rfind(' ', max_x);
                if (wrap_pos == string::npos)
                {
                    wrap_pos = max_x; // Force-wrap if no space is found
                }

                string overflow = buffer[i].substr(wrap_pos);
                buffer[i] = buffer[i].substr(0, wrap_pos);
                buffer.insert(buffer.begin() + i + 1, overflow);

                // Adjust the cursor if necessary
                if (cursor_y == i && cursor_x >= wrap_pos)
                {
                    cursor_y++;
                    cursor_x -= wrap_pos;
                    if (cursor_y >= view_start + max_y - 2)
                    {
                        view_start++;
                    }
                }
                else if (cursor_y > i)
                {
                    cursor_y++;
                    if (cursor_y >= view_start + max_y - 2)
                    {
                        view_start++;
                    }
                }
            }
        }
    }
};

int main(int argc, char *argv[])
{
    _argc = argc;
    _argv = argv;

    TextEditor editor;
    editor.run();
    return 0;
}
