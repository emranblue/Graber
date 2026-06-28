# Clipboard Graber

A robust C++ Qt GUI application to capture clipboard text and images, organize them under subjects, and append/edit content inside specific heading or subheading sections of your Markdown notes.

## Features

- **Dynamic Subject Management**: Create and open Markdown note files dynamically directly from the interface.
- **Hierarchical Content Selection**:
  - Automatically parses note files to list all headings (`<h2>`) and subheadings (`<h3>`) in a hierarchical dropdown.
  - Choose to append captured content at the end of the file or target a specific heading/subheading section.
- **HTML Anchors and IDs**: Inject unique HTML `id` attributes to headings and subheadings for seamless navigation.
- **Automated Document Normalizer**: Automatically converts standard Markdown headings to HTML headings with container boxes and IDs when note files are loaded or modified.
- **Structure Outline Trees (`.tree`)**: Generates matching `.tree` outline files to map note contents and optimize editing speeds.
- **Table of Contents (TOC) Generation**: Generates an index table grouped by category with page numbers and links to sections.
- **Section Categorization**: Map clipboard captures to categories like Environment, Energy, Economy, Law, Politics, etc.
- **Safe Section Deletion**: Delete any heading or subheading section from within the app. Deleted content blocks are safely backed up in the `~/GraberNotes/deleted/` directory.

## Prerequisites

You need a C++ compiler, CMake, and the Qt 6 development libraries.

### On Debian/Ubuntu:
```bash
sudo apt-get update && sudo apt-get install build-essential cmake qt6-base-dev
```

### On Fedora/RedHat:
```bash
sudo dnf install gcc-c++ cmake qt6-qtbase-devel
```

## How to Build

1. **Navigate to the project directory:**
   ```bash
   cd /home/emran/Desktop/extra/graber
   ```

2. **Configure the build with CMake:**
   ```bash
   cmake .
   ```

3. **Compile the application:**
   ```bash
   make
   ```

## How to Run

After a successful build, run the executable:
```bash
./graber
```

## How to Use

1. **Select a Subject**: Choose an existing subject note or create a new one using **"New Subject"**.
2. **Select Target Heading**: Choose a specific topic or subheading to append content to, or select `(Append to End)` to create a new section.
3. **Format & Section**: Select whether clipboard content should be appended as a Bullet Point, Main Heading, or Subheading. Assign the section category.
4. **Monitor/Append**:
   - Click **"Start"** to automatically monitor and append clipboard content.
   - Click **"Append"** next to the heading dropdown to manually insert current clipboard content to the selected heading.
5. **Delete Section**: Select any heading or subheading and click **"Delete"** to remove it. You can find the backup in `~/GraberNotes/deleted/` if needed.
