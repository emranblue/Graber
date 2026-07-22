# Clipboard Graber

A robust and feature-rich C++ Qt GUI application designed to capture clipboard text and images, organize them under subjects, and automatically append, edit, and reorganize content inside specific headings or subheading sections of your Markdown notes.

It is particularly useful for researching, study planning, document curation, or exam preparation (such as BCS candidates in Bangladesh) who need a rapid way to save, classify, and organize key information into structured markdown databases.

---

## Key Features

- **Dynamic Subject Management**:
  - Automatically lists existing `.md` subject notes from the dedicated home folder (`~/GraberNotes`).
  - Create and open new subject files dynamically directly from the GUI interface.
  - Quick access button to open the active markdown note in your default system editor.
  - **Toggle Subject Button & Shortcut**: Quickly cycle through available subjects via the "বিষয় পরিবর্তন" button or `Ctrl+Shift+E` shortcut.

- **Advanced Search & Heading Selection Dialog**:
  - Instantly search all headings and subheadings within your note file through a dedicated, interactive popup dialog.
  - **Multi-keyword filtering**: Filter headings using multiple whitespace-separated terms for highly specific targeting.
  - **Match Highlighting**: Highlight query terms within the results for clear visibility.
  - **Visual Indentation**: Clearly identifies subheadings (`<h3>`) with a sub-arrow (`↳`) indented below their parent headings (`<h2>`).
  - **Full Keyboard Accessibility**: Type to search, navigate selection using `Up/Down` arrow keys, and press `Enter` to select without touching your mouse.
  - **Mouse Click Propagation**: Click anywhere on custom list items to trigger selection.

- **Automated Document Normalizer**:
  - Automatically cleans and formats note files upon loading or modification.
  - Standardizes traditional Markdown headings (`##` and `###`) into structured HTML headings (`<h2>` and `<h3>`) with unique, auto-generated slug `id` and `data-section` attributes for anchors and links.
  - Automatically cleans legacy or container boxes (`<div ...>`) to maintain a standard, clean document layout.

- **Structured Table of Contents (TOC) Generation**:
  - Scans your note files and automatically injects a beautiful Table of Contents (TOC) at the top of the file, bounded by `<!-- TOC_START -->` and `<!-- TOC_END -->` comments.
  - Group headings by category/section (e.g., Environment, Energy, Economy, Law, etc.).
  - Generates clickable local links to headings, labeled with their heading index number, date of capture, and slug ID.

- **Section Categorization & Custom Sections**:
  - Map captured content to standard sections like Environment, Energy, Economy, Law-Constitution, Politics, Foreign-Policy, etc.
  - **Per-Subject Section Configurations (`<subject>.ini`)**: Each subject manages its own list of section categories in a dedicated `.ini` file (e.g., `~/GraberNotes/BCS/Bangla.ini`), allowing custom section lists per subject note.
  - Create **new custom categories** directly inside the app using the "New Section" feature. Newly created categories are automatically persisted into the active subject's `.ini` file, recognized in notes, and grouped in the TOC.

- **Flexible Clipboard Capture Modes**:
  - Toggle monitoring between **Copy Mode** (Ctrl+C / Clipboard) and **Select Mode** (Primary Selection).
  - Format clipboard captures as a **Bullet Point**, a **Main Heading** (styled Red `<h2>`), or a **Subheading** (styled Blue `<h3>`).
  - **Manual Append**: Append current clipboard contents directly to the selected target heading manually.
  - **Inject Heading**: Inject clipboard text as a new main heading at the end of the note file.

- **Section Move & Relocation (Shift Heading)**:
  - Reorganize notes on the fly! Select any section and shift/move the entire block (including all its subheadings and bullets) to sit under a different heading or at the very end of the file.

- **Outline Structure Trees (`.tree`)**:
  - Automatically generates an outline mapping file (e.g., `subject.tree`) alongside your Markdown note. It details the hierarchical structure of your notes grouped by category sections.

- **Safe Section Deletion**:
  - Delete any heading or subheading section from within the app.
  - Bounded content is not lost; deleted blocks are automatically backed up as `.txt` files in `~/GraberNotes/deleted/` with timestamping for easy recovery.

- **Clipboard Image Import**:
  - Save images copied to your clipboard directly into your notes directory as PNGs (`~/GraberNotes/images/`).
  - Automatically appends a markdown image link to the active date section of the file.

- **Custom Keyboard Shortcuts & Settings**:
  - Supports 13 global actions (Start/Stop monitoring, Add image, New subject, Open active note, Append to heading, Inject heading, Shift heading section, Delete heading section, Add custom section, Toggle capture format, Toggle section category, Toggle subject selection) mapped to keyboard shortcuts.
  - Remap shortcuts using a dedicated **Settings Dialog** inside the app.
  - Settings are persisted across sessions in `~/GraberNotes/settings.ini`.

---

## Directory Structure

The application creates and manages files under the `~/GraberNotes` directory:
- `~/GraberNotes/` - Main note directory containing your Markdown (`.md`), Structure Tree (`.tree`), and per-subject section configuration (`.ini`) files.
- `~/GraberNotes/images/` - Contains captured images referenced in your notes.
- `~/GraberNotes/deleted/` - Backup folder for deleted sections.
- `~/GraberNotes/backup/` - Contains post-build executable backups (Linux/macOS).
- `~/GraberNotes/settings.ini` - Persistent shortcut settings configuration file.
- `~/GraberNotes/<subject>.ini` - Per-subject section configuration settings file.
- `~/GraberNotes/debug.log` - Application activity and error logs.

---

## Prerequisites & Installation

To build and run Clipboard Graber, you need a C++ compiler supporting C++17, CMake (version 3.16+), and Qt 6 development libraries.

### 1. Linux

#### Installing Prerequisites:
* **Debian / Ubuntu / Linux Mint:**
  ```bash
  sudo apt update && sudo apt install build-essential cmake qt6-base-dev
  ```
* **Fedora / RHEL / CentOS:**
  ```bash
  sudo dnf install gcc-c++ cmake qt6-qtbase-devel
  ```
* **Arch Linux / Manjaro:**
  ```bash
  sudo pacman -S base-devel cmake qt6-base
  ```

#### Building from Source:
1. Open a terminal and navigate to the project directory:
   ```bash
   cd path/to/graber
   ```
2. Configure the build:
   ```bash
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
   ```
3. Compile the application:
   ```bash
   cmake --build build
   ```
4. Run the executable:
   ```bash
   ./build/graber
   ```

---

### 2. Windows

#### Installing Prerequisites:
1. Download and install **Visual Studio 2022** (Community Edition is free). Choose the **Desktop development with C++** workload during installation.
2. Download and install **Qt 6** (e.g. Qt 6.8.x) from the [Qt Online Installer](https://www.qt.io/download-open-source). Ensure you check the MSVC compilation kit component (e.g., `MSVC 2022 64-bit`).
3. Download and install **CMake**.

#### Building from Source:
1. Open the **Developer Command Prompt for VS 2022**.
2. Navigate to your project directory:
   ```cmd
   cd path\to\graber
   ```
3. Configure the build with CMake (specify your Qt6 directory if CMake cannot auto-find it, e.g. `-DCMAKE_PREFIX_PATH=C:\Qt\6.8.1\msvc2022_64`):
   ```cmd
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
   ```
4. Compile the application:
   ```cmd
   cmake --build build --config Release
   ```
5. Deploy required Qt DLLs (runs dynamic deployment to make the binary portable):
   ```cmd
   mkdir dist
   copy build\Release\graber.exe dist\
   windeployqt --dir dist dist\graber.exe
   ```
6. Run the application from the `dist/` directory by double-clicking `graber.exe`.

---

### 3. macOS

#### Installing Prerequisites:
1. Install **Xcode Command Line Tools** by running:
   ```bash
   xcode-select --install
   ```
2. Install **Homebrew** if not already installed, then install CMake and Qt 6:
   ```bash
   brew install cmake qt
   ```

#### Building from Source:
1. Navigate to the project directory:
   ```bash
   cd path/to/graber
   ```
2. Configure the build (Homebrew installs Qt in a prefix path that needs to be passed to CMake):
   ```bash
   cmake -B build -S . -DCMAKE_PREFIX_PATH=$(brew --prefix qt) -DCMAKE_BUILD_TYPE=Release
   ```
3. Compile the application:
   ```bash
   cmake --build build
   ```
4. Standalone app bundle deployment (packages all libraries/frameworks inside the `.app` bundle):
   ```bash
   macdeployqt build/graber.app -dmg
   ```
5. Run the application or mount the generated `.dmg` file to install it:
   ```bash
   open build/graber.app
   ```

---

### 4. FreeBSD

#### Installing Prerequisites:
Install build utilities, compiler, CMake, and the Qt 6 library package:
```bash
sudo pkg install cmake qt6
```

#### Building from Source:
1. Navigate to the project directory:
   ```bash
   cd path/to/graber
   ```
2. Configure the build:
   ```bash
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
   ```
3. Compile the application:
   ```bash
   cmake --build build
   ```
4. Run the executable:
   ```bash
   ./build/graber
   ```

---

## How to Use

1. **Select a Subject**: Choose an existing subject note from the dropdown or create a new one using **"New Subject"**.
2. **Select Target Heading**: Click the target heading button (shows "(Append to End)" by default) to open the searchable popup. Type keywords to search, navigate with Arrow Keys, and press Enter to select.
3. **Configure & Start Monitoring**:
   - Select format (Point, Main Heading, Subheading).
   - Select the section category (or add a new one with "New Section").
   - Click **"Start"** to automatically monitor and append clipboard content (or use shortcut `Ctrl+Shift+S`).
4. **Manual Actions**:
   - Click **"Append"** (`Ctrl+Shift+A`) to manually append clipboard text to the selected heading.
   - Click **"Inject"** (`Ctrl+Shift+J`) to insert clipboard text as a new main heading at the end of the note.
   - Click **"Add Image"** (`Ctrl+Shift+I`) to grab a clipboard image.
5. **Manage Sections**:
   - Select a heading/subheading in the popup.
   - Click **"Shift"** (`Ctrl+Shift+H`) to choose a new target position and move the entire section.
   - Click **"Delete"** (`Ctrl+Shift+D`) to remove the section. A backup of the deleted content is saved to `~/GraberNotes/deleted/`.
6. **Configure Shortcuts**: Click **"Settings"** to remap keys. Click **"Reset Defaults"** or **"Save"** to apply changes.
