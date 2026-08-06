# Developing History

This log documents recent development changes, feature updates, and UI refinements made to the **Clipboard Graber** application.

---

## Recent Updates & Refinements

### 1. Wizard & Tool Button UI Updates
- **Button Renaming & Relabeling**:
  - Renamed button text from `"উইজার্ড ও টুলস"` ("Wizards & Tools") to **`"টুলস"`** ("Tools") in [`ui/panels/ControlsBar.cpp`](file:///home/emran/Desktop/Graber-main-fixed/ui/panels/ControlsBar.cpp).
  - Updated tooltips, menu titles (`"টুলস (Tools)"`), warning dialog titles, and functional action registrations in [`ui/controllers/ClipboardGrabber_Dialogs.cpp`](file:///home/emran/Desktop/Graber-main-fixed/ui/controllers/ClipboardGrabber_Dialogs.cpp) and [`ui/controllers/ClipboardGrabber_Actions.cpp`](file:///home/emran/Desktop/Graber-main-fixed/ui/controllers/ClipboardGrabber_Actions.cpp).
- **Button Icon**:
  - Replaced icon glyph with a wrench/tool icon (`QChar(0xe9e9)`) using the embedded Feather icon renderer.

### 2. Child Window & Popup Menu Styling
- **Clean White Theme (`#ffffff`)**:
  - Updated background color of the child wizard dialog ([`ExportNoteWizard.cpp`](file:///home/emran/Desktop/Graber-main-fixed/ui/dialogs/ExportNoteWizard.cpp)) to `#ffffff`.
  - Configured clean white background (`#ffffff`) with subtle rounded borders, hover highlights, and standard separators for the popup `QMenu` triggered by the Tools button.

### 3. Subject & Folder Navigation Filtering
- **Hidden `config` System Folder**:
  - Filtered `"config"` and `"config/"` subpaths out of `populateFoldersFromDisk()` in [`repositories/NoteRepository.cpp`](file:///home/emran/Desktop/Graber-main-fixed/repositories/NoteRepository.cpp) so the system configuration directory does not appear in the folder dropdown list.
  - Filtered `"config"` and `"config/"` out of `populateSubjectsFromDisk()` in [`repositories/NoteRepository_Subjects.cpp`](file:///home/emran/Desktop/Graber-main-fixed/repositories/NoteRepository_Subjects.cpp) so internal configuration files are excluded from subject navigation.

---

## Build Verification
- Verified process compilation via `make` — all targets built cleanly with code `0`.
