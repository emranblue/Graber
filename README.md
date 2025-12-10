# Clipboard Graber

A simple C++ Qt GUI application to capture copied text and append it to Markdown files.

## Prerequisites

You need a C++ compiler, CMake, and the Qt 6 development libraries.

### On Debian/Ubuntu:

You can install all the necessary dependencies with the following command:

```bash
sudo apt-get update && sudo apt-get install build-essential cmake qt6-base-dev
```

### On other systems (Fedora, Arch, etc.):

Use your system's package manager to install `cmake`, a C++ compiler (like `g++`), and the Qt 6 development packages (e.g., `qt6-qtbase-devel`).

## How to Build

1.  **Navigate to the project directory:**
    Open a terminal and `cd` into the directory containing `main.cpp` and `CMakeLists.txt`.

    ```bash
    cd /path/to/your/project/directory
    ```

2.  **Configure the build with CMake:**
    This command generates the build files (e.g., Makefiles).

    ```bash
    cmake .
    ```

3.  **Compile the application:**
    This command builds the executable file named `graber`.

    ```bash
    make
    ```

## How to Run

After a successful build, you can run the application directly from the terminal:

```bash
./graber
```

### How it Works:

1.  Run the application.
2.  Select a target file by clicking either **"Use 'Bangladesh.md'"** or **"Use 'International.md'"**.
3.  Click **"Start"** to begin monitoring.
4.  Now, select and copy any sentence from any window on your computer.
5.  The application will automatically grab the copied text and append it to the selected Markdown file in a boxed format.
6.  Click **"Stop"** to pause the monitoring.
7.  The Markdown files (`Bangladesh.md`, `International.md`) will be created in the same directory where you run the `graber` executable.
