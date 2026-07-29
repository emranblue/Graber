# Project Directory Tree

```
graber/
├── CMakeLists.txt                 # Main CMake build configuration
├── Makefile                       # Cross-platform Makefile wrapper
├── README.md                      # Project documentation and usage guide
├── PROJECT_TREE.md                # Detailed project architecture & directory structure
├── main.cpp                       # Application entry point & Qt application bootstrap
├── resources.qrc                  # Qt Resource file (fonts, icons)
├── Kalpurush.ttf                  # Bangla font asset
├── feather.ttf                    # Vector icon font asset
│
├── core/                          # Domain Models & Core Types
│   └── Types.h                    # Data structures (NoteItem, SectionItem, ShortcutConfig, Enums)
│
├── interfaces/                    # Abstract Contracts (SOLID / Dependency Inversion)
│   ├── IClipboardMonitor.h        # Interface for clipboard & selection monitoring
│   ├── IDocumentFormatter.h       # Interface for document parsing, slugification, & TOC engines
│   ├── INoteRepository.h          # Interface for note storage & section management
│   └── ISectionRepository.h       # Interface for per-subject section settings persistence
│
├── services/                      # Application Service Layer
│   ├── NoteService.h              # Note application service orchestrating domain operations
│   └── NoteService.cpp
│
├── repositories/                  # Storage & Data Persistence Layer
│   ├── NoteRepository.h           # File-based note repository implementation
│   ├── NoteRepository.cpp
│   ├── IniSectionRepository.h     # INI-based section configuration repository
│   └── IniSectionRepository.cpp
│
├── formatters/                    # Document Processing & Normalization
│   ├── MarkdownDocumentFormatter.h# Markdown & HTML heading normalizer & TOC generator
│   ├── MarkdownDocumentFormatter.cpp
│   ├── MarkdownUtils.h            # Text parsing, slug generation, & bound detection utilities
│   └── MarkdownUtils.cpp
│
├── monitors/                      # System Clipboard & Selection Tracking
│   ├── ClipboardMonitor.h         # Qt clipboard poller implementation
│   └── ClipboardMonitor.cpp
│
├── shortcuts/                     # Keyboard Shortcuts System
│   ├── ShortcutManager.h          # Shortcut manager & hotkey binder
│   └── ShortcutManager.cpp
│
├── ui/                            # Qt User Interface Components & Dialogs
│   ├── ClipboardGrabber.h         # Main window controller
│   ├── ClipboardGrabber.cpp
│   ├── ClipboardGrabberUI.h       # UI layout setup & widget references
│   ├── ClipboardGrabberUI.cpp
│   ├── HeadingSelectDialog.h      # Interactive heading search & selection dialog
│   ├── HeadingSelectDialog.cpp
│   ├── ShortcutsSettingsDialog.h  # Shortcut configuration settings dialog
│   └── ShortcutsSettingsDialog.cpp
│
└── utils/                         # Helper Utilities
    ├── Utils.h                    # Styling helpers, icon generators, & debug logging
    └── Utils.cpp
```
