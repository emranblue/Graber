# Graber — Project Directory Tree

```
Graber/
├── CMakeLists.txt                      # CMake build config (targets Qt6: Core, Gui, Widgets)
├── Makefile                            # Convenience wrapper around the CMake build
├── README.md                           # Project overview & usage guide
├── UPDATE_GUIDE.md                     # Notes on updating/upgrading the app
├── PROJECT_TREE.md                     # This file
├── main.cpp                            # App entry point: QApplication, global stylesheet, font loading
├── main.cpp.bak                        # Backup of an earlier, pre-refactor main.cpp
├── resources.qrc                       # Qt resource file (fonts + icons bundled into the binary)
├── Kalpurush.ttf                       # Bangla UI font
├── feather.ttf                         # Feather icon font (used for in-app glyph icons)
│
├── core/                               # Shared low-level types & compatibility shims
│   ├── Types.h                         # NoteItem, SectionItem, ShortcutConfig, shared enums
│   └── QtFixes.h                       # Small cross-version Qt compatibility fixes
│
├── interfaces/                         # Abstract contracts (dependency inversion / SOLID)
│   ├── IAction.h                       # Command/Action pattern contract
│   ├── IClipboardMonitor.h             # Clipboard & selection monitoring contract
│   ├── IDocumentFormatter.h            # Markdown/heading formatting contract
│   ├── INoteRepository.h               # Note storage contract
│   ├── INoteService.h                  # Note application-service contract
│   ├── ISectionRepository.h            # Per-subject section settings contract
│   ├── IServiceRegistry.h              # Service locator contract
│   └── IWizardFeature.h                # Extensible wizard/tool plugin contract
│
├── services/                           # Application service layer & registries
│   ├── NoteService.{h,cpp}             # Implements INoteService — note read/write/append logic
│   ├── ServiceRegistry.{h,cpp}         # Central service locator
│   ├── ActionRegistry.{h,cpp}          # Central command registry; binds UI buttons to actions
│   └── FeatureManager.{h,cpp}          # Registry/dispatcher for wizards & extensions
│
├── repositories/                       # Storage & persistence layer
│   ├── NoteRepository.{h,cpp}          # File-based note storage on disk
│   └── IniSectionRepository.{h,cpp}    # INI-based per-subject section config storage
│
├── formatters/                         # Document parsing & normalization
│   ├── MarkdownDocumentFormatter.{h,cpp} # Heading normalization, TOC generation
│   └── MarkdownUtils.{h,cpp}           # Slug generation, section-bound detection helpers
│
├── monitors/                           # System clipboard tracking
│   └── ClipboardMonitor.{h,cpp}        # Polls the system clipboard/selection for changes
│
├── shortcuts/                          # Keyboard shortcuts (in-app + system-wide)
│   └── ShortcutManager.{h,cpp}         # QShortcut bindings + cross-platform global hotkeys
│                                        #   (Win32 RegisterHotKey / X11 XGrabKey)
│
├── ui/                                 # Qt widgets & dialogs
│   ├── ClipboardGrabber.{h,cpp}        # Main window logic: start/stop capture, event handling
│   ├── ClipboardGrabberUI.{h,cpp}      # Main window layout/widget construction
│   ├── ExportNoteWizard.{h,cpp}        # Wizard dialog for exporting notes
│   ├── HeadingSelectDialog.{h,cpp}     # Dialog for searching/selecting a target heading
│   └── ShortcutsSettingsDialog.{h,cpp} # Dialog for viewing/editing shortcut bindings
│
├── utils/                              # Misc shared helpers
│   └── Utils.{h,cpp}                   # Feather-icon rendering, general utility functions
│
└── resources/                          # Packaged icons & platform metadata
    ├── graber.rc                       # Windows resource script (.exe icon embedding)
    └── icons/
        ├── app.ico, app-1024.png       # Application icon
        ├── file.ico, file-1024.png     # Note/file icon
        └── folder.ico, folder-1024.png # Folder icon
```

## Architecture at a glance

- **Layered / clean-architecture style**: `interfaces/` defines contracts, `services/` implements
  application logic against those contracts, `repositories/` handles persistence, and `ui/` only
  talks to services/registries — never directly to file I/O.
- **`ServiceRegistry`** acts as a simple service locator so components can be swapped without
  rewiring constructors everywhere.
- **`ActionRegistry`** centralizes every user-triggerable action (start/stop, add subject, inject
  heading, etc.) with an enabled/disabled predicate, and binds them to both UI buttons and
  keyboard shortcuts from a single source of truth.
- **`ShortcutManager`** layers two shortcut systems: in-app `QShortcut`s (work while the window is
  focused) and true system-wide global hotkeys (work from any app) via native Win32/X11 APIs.
