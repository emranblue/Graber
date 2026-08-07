# Developer Guide: Table of Contents (TOC) Architecture & Refinement

## Overview
This document details the architectural design, bug findings, root cause analysis, and enhancement specifications for the Table of Contents (TOC) system in the **Clipboard Graber** application.

---

## 1. TOC Architecture in Codebase

The TOC subsystem is responsible for scanning note contents, discovering main headings and subheadings, extracting date metadata and section groupings, and embedding a Table of Contents bounded by `<!-- TOC_START -->` and `<!-- TOC_END -->` comments at the top of note files.

### Key Components:
- **`formatters/MarkdownDocumentFormatter_Toc.cpp`**:
  - `generateToc(...)`: Extracts the TOC block from processed note content.
  - `updateTocInContent(...)`: Scans note body line-by-line, strips old TOC blocks, identifies `<h2>`, `<h3>`, `##`, `###` headings and date markers, normalizes attributes (`id`, `data-section`, `style`), and injects the regenerated TOC.
- **`formatters/MarkdownDocumentFormatter_TocHelpers.h` & `MarkdownDocumentFormatter_TocHelpers.cpp`**:
  - `stripExistingToc(...)`: Removes existing `<!-- TOC_START --> ... <!-- TOC_END -->` blocks.
  - `extractPreview(...)` & `plainExcerpt(...)`: Generates clean excerpt previews for TOC entries.
  - `buildTocBlock(...)`: Formats heading data into the final TOC block structure.
- **`repositories/NoteRepository_Content.cpp`**:
  - `updateTocInFile(...)`: Reads note content from disk, invokes `formatter_->updateTocInContent(...)`, and performs atomic file writes (`writeTextAtomic`).
- **`ui/controllers/ClipboardGrabber_Capture.cpp` & `ClipboardGrabber_Headings.cpp`**:
  - Triggers `note_service_.updateTocInFile(...)` whenever content, headings, or images are injected.

---

## 2. Root Cause Analysis: Why Headings & Subheadings Were Failing to Append

### Bug 1: Dropped Subheadings (Level 3 Mismatch & Orphan Subheadings)
In `MarkdownDocumentFormatter_TocHelpers.cpp` (`buildTocBlock`):
- TOC block generation previously iterated over main headings (`level == 2`), and then scanned subsequent items for level 3 subheadings matching `s.parent_slug == h.slug`.
- **Root Cause & Failure Modes**:
  1. **Subheadings before Main Headings**: If subheadings (`###` or `<h3>`) appeared before any `<h2>` heading in a file, `parent_slug` was empty. These subheadings were completely omitted from `buildTocBlock`.
  2. **Subheading-only Notes**: If a note file contained only subheadings (`###` / `<h3>`) without an `<h2>` main heading, `mainIndices` was empty for all sections, resulting in an empty TOC or missing headings.
  3. **Slug Mismatch**: If heading text was edited or if slug generation differed between parent and child, `s.parent_slug != h.slug` caused the subheading to be skipped (`continue`). When the next `level == 2` heading was reached, the loop terminated (`break`), permanently discarding the subheading.

### Bug 2: Ignored Custom & Dynamic Sections
- `buildTocBlock` only iterated through sections passed in `effective_sections` (pre-defined UI section list).
- If a note contained headings assigned to custom or dynamically parsed sections (e.g. `data-section="physics"` or `<!-- section:physics -->`), those section headings were omitted from the TOC because they were not in `effective_sections`.

### Bug 3: Strict Heading Regex & Date Matching Vulnerabilities
- `md_sub_regex` (`^(#{3})\s+(?!\*\*\*)(.*?)$`) excluded `### ***` (date pattern). If a date stamp was written slightly differently (or without `***`), or if a subheading contained bold/italic formatting at the beginning, matching failed or identified date lines as subheadings.
- `h2_regex` and `h3_regex` stripped tags improperly when titles contained nested HTML elements (e.g. `<b>`, `<span>`, `<code>`).

### Bug 4: Incomplete Heading State Refresh in UI
- In `ClipboardGrabber_Capture.cpp` (`write_to_file`), `populate_headings_from_file()` was only called when `format_index == 1` (Main Heading). When users inserted subheadings (`format_index == 2`), `all_headings_` in memory was not refreshed, leading to out-of-sync dropdowns and heading selection states.

---

## 3. Glossy & Modern TOC Design Specifications

To deliver a premium, visually stunning user experience, the Table of Contents has been upgraded from plain plain-text markdown list items to a modern, glossy HTML card design:

1. **Glassmorphism Container**:
   - Translucent background with sleek gradient (`linear-gradient(135deg, rgba(255,255,255,0.95), rgba(240,244,248,0.95))`).
   - Soft blue accent border (`rgba(0, 122, 255, 0.2)`), rounded corners (`border-radius: 12px`), and dynamic depth shadow (`0 8px 32px 0 rgba(31, 38, 135, 0.07)`).
2. **Header Title Banner**:
   - Premium badge design with icon (`📌 সূচিপত্র · Table of Contents`).
   - Gradient line separator dividing sections.
3. **Section Category Cards & Badges**:
   - Each section (e.g. *বাংলা*, *English*, *সাধারণ জ্ঞান*, *অন্যান্য*) features a pill badge with color accent.
4. **Hierarchical Items & Subheadings**:
   - Main headings feature numbered index badges (`1`, `2`, `3...`) with bold interactive anchor links (`<a href="#slug">`).
   - Subheadings feature nested indentations, sub-numbering (`1.1`, `1.2...`), subtle date tags (`📅 07 August, 2026`), and muted italic excerpt previews.
5. **Universal Viewer Compatibility**:
   - Bounded by `<!-- TOC_START -->` and `<!-- TOC_END -->`.
   - Maintains standard HTML anchor links (`href="#slug"`) matching `<h2 id="slug">` and `<h3 id="slug">`, ensuring full smooth scrolling functionality in both Qt WebEngine, Markdown HTML view, and standard Markdown editors.

---

## 4. Summary of Code Fixes

1. **`formatters/MarkdownDocumentFormatter_TocHelpers.cpp`**:
   - Refactored `buildTocBlock` to automatically collect all section slugs present in `headings`.
   - Restructured heading hierarchy logic: renders main headings, nested subheadings, AND orphan subheadings (subheadings without a level 2 parent). Guarantees **100% of headings and subheadings appear in the TOC**.
   - Built modern glossy HTML template styling for TOC block generation.
2. **`formatters/MarkdownDocumentFormatter_Toc.cpp`**:
   - Improved regex parsing for HTML (`<h2>`, `<h3>`) and Markdown (`##`, `###`) headings.
   - Cleaned HTML tags from title text prior to slug generation and excerpt previewing.
3. **`ui/controllers/ClipboardGrabber_Capture.cpp`**:
   - Updated `write_to_file` to call `populate_headings_from_file()` for both Main Headings (`format_index == 1`) and Subheadings (`format_index == 2`).
4. **Image Path & Heading Insertion Engine (`INoteRepository`, `NoteRepository_Content`, `ClipboardGrabber_Capture`)**:
   - Fixed file links in project documentation (`developing_history.md`, `README.md`, `PROJECT_TREE.md`) to resolve path mismatches.
   - Enhanced `writeImageToNote` to support target heading selection (`selectedSlug`). Images are now inserted under the user's selected target heading if chosen, or appended under the active date section.
5. **Active Template Formatting & Timeline Line Break Fix (`config/templates.json`, `MarkdownDocumentFormatter_Toc`)**:
   - Removed extra trailing newline in `timeline` mind map template body in project `config/templates.json` and `~/GraberNotes/config/templates.json`.
   - Updated `MarkdownDocumentFormatter_Toc.cpp` to format main headings and subheadings using `MarkdownTemplateManager` active `templates.json` templates.
   - Batch processed all 9 markdown notes in `~/GraberNotes` using `./build/update_all_notes`.
6. **Absolute Image Path Based on `GraberNotes` Folder (`ClipboardGrabber_Capture`, `NoteRepository_Content`)**:
   - Centralized image capture savings to root `~/GraberNotes/images/`.
   - Formatted image Markdown links using absolute paths based on `notes_dir_path_ + "/images/"` (e.g. `![Image](/home/emran/GraberNotes/images/<filename>)`).
   - Automatically normalized existing relative image tags across all `.md` files in subfolders, ensuring images render properly regardless of folder moves.

---
