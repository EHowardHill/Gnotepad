# Gnotepad

Gnotepad is an extremely simple and extremely lightweight text editor. It's designed to be like 'Vi' for people who are afraid of Vi, and it's optimized for note-taking.

Gnotepad automatically saves upon keypress and uses Ctrl+C to terminate the application.

**Installation:**

**Ubuntu/Debian Systems:**

1. **Install Build Essentials:**
   ```sudo apt install build-essential```

2. **Install ncurses libraries:**
    ```sudo apt install libncurses5-dev libncursesw5-dev```

3. **Build:**
    ```make```

4. **Run:**
    ```./gnotepad```

You may also provide the filename to open/save as an argument. Otherwise, it will use "output.txt" as a default.

    ```./gnotepad myfile.txt```
