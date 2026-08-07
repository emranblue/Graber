# Developing History

This log documents recent development changes, feature updates, and UI refinements made to the **Clipboard Graber** application.

---

## Recent Updates & Refinements

### 1. Wizard & Tool Button UI Updates
- **Button Renaming & Relabeling**:
  - Renamed button text from `"উইজার্ড ও টুলস"` ("Wizards & Tools") to **`"টুলস"`** ("Tools") in [`ui/panels/ControlsBar.cpp`](file:///home/emran/Desktop/extra/graber/ui/panels/ControlsBar.cpp).
  - Updated tooltips, menu titles (`"টুলস (Tools)"`), warning dialog titles, and functional action registrations in [`ui/controllers/ClipboardGrabber_Dialogs.cpp`](file:///home/emran/Desktop/extra/graber/ui/controllers/ClipboardGrabber_Dialogs.cpp) and [`ui/controllers/ClipboardGrabber_Actions.cpp`](file:///home/emran/Desktop/extra/graber/ui/controllers/ClipboardGrabber_Actions.cpp).
- **Button Icon**:
  - Replaced icon glyph with a wrench/tool icon (`QChar(0xe9e9)`) using the embedded Feather icon renderer.

### 2. Child Window & Popup Menu Styling
- **Clean White Theme (`#ffffff`)**:
  - Updated background color of the child wizard dialog ([`ExportNoteWizard.cpp`](file:///home/emran/Desktop/extra/graber/ui/dialogs/ExportNoteWizard.cpp)) to `#ffffff`.
  - Configured clean white background (`#ffffff`) with subtle rounded borders, hover highlights, and standard separators for the popup `QMenu` triggered by the Tools button.

### 3. Subject & Folder Navigation Filtering
- **Hidden `config` System Folder**:
  - Filtered `"config"` and `"config/"` subpaths out of `populateFoldersFromDisk()` in [`repositories/NoteRepository.cpp`](file:///home/emran/Desktop/extra/graber/repositories/NoteRepository.cpp) so the system configuration directory does not appear in the folder dropdown list.
  - Filtered `"config"` and `"config/"` out of `populateSubjectsFromDisk()` in [`repositories/NoteRepository_Subjects.cpp`](file:///home/emran/Desktop/extra/graber/repositories/NoteRepository_Subjects.cpp) so internal configuration files are excluded from subject navigation.

### 4. Table of Contents (TOC) Architecture & Glossy Design Refinement
- **Developer Guide**:
  - Created [`DEVELOPER_GUIDE.md`](file:///home/emran/Desktop/extra/graber/DEVELOPER_GUIDE.md) detailing TOC architecture, root cause analysis, bug diagnostics, design specifications, and extension points.
- **Fail-Safe Heading & Subheading Inclusion**:
  - Refactored `buildTocBlock` in [`MarkdownDocumentFormatter_TocHelpers.cpp`](file:///home/emran/Desktop/extra/graber/formatters/MarkdownDocumentFormatter_TocHelpers.cpp) to guarantee that all headings (`<h2>`, `##`) and subheadings (`<h3>`, `###`) are rendered, including orphan subheadings and custom sections.
- **Glossy Glassmorphism Design**:
  - Transformed TOC from plain Markdown bullet list to a glossy modern card container with soft gradient background, blue accent border, rounded corners, section category badges, index numbers, interactive anchor links (`<a href="#slug">`), date metadata tags, and excerpt previews.
- **Heading Cleaning & UI Refresh**:
  - Sanitized HTML/Markdown tags in [`MarkdownDocumentFormatter_Toc.cpp`](file:///home/emran/Desktop/extra/graber/formatters/MarkdownDocumentFormatter_Toc.cpp) before slug generation and excerpt previewing.
  - Updated `write_to_file` in [`ui/controllers/ClipboardGrabber_Capture.cpp`](file:///home/emran/Desktop/extra/graber/ui/controllers/ClipboardGrabber_Capture.cpp) to refresh heading states for both Main Headings (`format_index == 1`) and Subheadings (`format_index == 2`).

### 5. Image Path & Heading Insertion Engine Refinement
- **Markdown File Link Resolution**:
  - Corrected local file links in [`developing_history.md`](file:///home/emran/Desktop/extra/graber/developing_history.md) to point to `/home/emran/Desktop/extra/graber` instead of legacy path `Graber-main-fixed`.
  - Updated developer guide file references in [`README.md`](file:///home/emran/Desktop/extra/graber/README.md) and [`PROJECT_TREE.md`](file:///home/emran/Desktop/extra/graber/PROJECT_TREE.md) from `UPDATE_GUIDE.md` to [`DEVELOPER_GUIDE.md`](file:///home/emran/Desktop/extra/graber/DEVELOPER_GUIDE.md).
- **Target Heading Insertion for Images**:
  - Updated `writeImageToNote` in [`INoteRepository.h`](file:///home/emran/Desktop/extra/graber/interfaces/INoteRepository.h), [`INoteService.h`](file:///home/emran/Desktop/extra/graber/interfaces/INoteService.h), [`NoteRepository_Content.cpp`](file:///home/emran/Desktop/extra/graber/repositories/NoteRepository_Content.cpp), and [`ClipboardGrabber_Capture.cpp`](file:///home/emran/Desktop/extra/graber/ui/controllers/ClipboardGrabber_Capture.cpp) to support `selectedSlug`.
  - When users capture/add a clipboard image while a heading is selected in the UI, the image markdown (`![Image](images/<filename>)`) is now inserted under the target heading rather than forced to the end of the file.

### 6. Active `templates.json` Formatting Sync & Timeline Line Break Fix
- **Timeline Extra Line Break Removal**:
  - Fixed trailing double newline (`\n\n`) in `timeline` mind map template body in both project [`config/templates.json`](file:///home/emran/Desktop/extra/graber/config/templates.json) and user configuration file `~/GraberNotes/config/templates.json`. Timeline nodes now join cleanly along their vertical border with necessary line breaks.
- **Active Template Heading Formatting**:
  - Updated [`MarkdownDocumentFormatter_Toc.cpp`](file:///home/emran/Desktop/extra/graber/formatters/MarkdownDocumentFormatter_Toc.cpp) to render main headings (`"heading"`) and subheadings (`"subheading"`) using `MarkdownTemplateManager` active format bodies.
  - Executed `./build/update_all_notes` to update all 9 note files under `~/GraberNotes` (`.md`, `.ini`, `.tree`), bringing all headings, subheadings, timeline nodes, and TOC blocks into 100% sync with the active `templates.json`.

### 7. Absolute Image Path Based on `GraberNotes` Folder
- **Centralized Image Savings**:
  - Updated `add_clipboard_image` in [`ui/controllers/ClipboardGrabber_Capture.cpp`](file:///home/emran/Desktop/extra/graber/ui/controllers/ClipboardGrabber_Capture.cpp) to save images directly into root `~/GraberNotes/images/`.
- **Absolute Image Link Generation**:
  - Updated `writeImageToNote` in [`repositories/NoteRepository_Content.cpp`](file:///home/emran/Desktop/extra/graber/repositories/NoteRepository_Content.cpp) to format image Markdown tags with absolute paths based on `notes_dir_path_ + "/images/"` (e.g. `![Image](/home/emran/GraberNotes/images/<filename>)`).
- **Batch Normalization for Existing Notes**:
  - Updated `NoteRepository::updateTocInFile` in [`repositories/NoteRepository_Content.cpp`](file:///home/emran/Desktop/extra/graber/repositories/NoteRepository_Content.cpp) to automatically convert relative image tags (`images/<filename>`) in all `.md` files to absolute paths (`<GraberNotes>/images/<filename>`).
  - Executed `./build/update_all_notes` to normalize all image paths across all `.md` files in `~/GraberNotes/`. Moving notes to new folders or subfolders now preserves 100% image visibility.

---

## Build Verification
- Verified process compilation via `make` — all targets built cleanly with code `0`.
