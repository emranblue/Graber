# Project Directory Tree

```
graber/
├── CMakeLists.txt
├── Makefile
├── README.md
├── PROJECT_TREE.md
├── UPDATE_GUIDE.md
├── main.cpp
├── resources.qrc
├── Kalpurush.ttf
├── feather.ttf
│
├── config/                         # Project-seeded JSON (also embedded in resources.qrc)
│   ├── templates.json              # Markdown & diagram format bodies → ~/GraberNotes/config/
│   └── shortcut_templates.json     # Default shortcut catalog (user overrides in settings.ini)
│
├── core/
│   ├── Types.h
│   └── QtFixes.h
│
├── interfaces/
│   ├── IAction.h
│   ├── IClipboardMonitor.h
│   ├── IDocumentFormatter.h
│   ├── INoteRepository.h
│   ├── INoteService.h
│   ├── ISectionRepository.h
│   ├── IServiceRegistry.h
│   └── IWizardFeature.h
│
├── services/
│   ├── NoteService.h / .cpp
│   ├── ServiceRegistry.h / .cpp
│   ├── ActionRegistry.h / .cpp
│   └── FeatureManager.h / .cpp
│
├── repositories/
│   ├── NoteRepository.h
│   ├── NoteRepository.cpp              # ctors, path, folders, sections
│   ├── NoteRepository_Subjects.cpp     # subjects list / create / move
│   ├── NoteRepository_Content.cpp      # normalize, TOC, parse, image, inject
│   ├── NoteRepository_Append.cpp       # append to heading, writeToNote
│   ├── NoteRepository_Diagram.cpp      # insert / upsert live diagrams
│   ├── NoteRepository_Heading.cpp      # delete / shift heading blocks
│   └── IniSectionRepository.h / .cpp
│
├── formatters/
│   ├── MarkdownDocumentFormatter.h
│   ├── MarkdownDocumentFormatter.cpp           # slug / diagram / tree / restore
│   ├── MarkdownDocumentFormatter_Normalize.cpp # heading normalize
│   ├── MarkdownDocumentFormatter_Toc.cpp       # TOC build & inject
│   ├── MarkdownDocumentFormatter_Parse.cpp     # structure → NoteItem list
│   ├── MarkdownUtils.h / .cpp                    — slug, tree, display ids
│   ├── MarkdownUtils_Bounds.cpp                  — heading/subheading bounds
│   ├── DiagramTemplates.h / .cpp
│   ├── CaptureContentFormatter.h / .cpp        # formatIndex → escaped markup
│   └── MarkdownTemplateManager.h / .cpp        # loads ~/GraberNotes/templates.txt live
│
├── monitors/
│   └── ClipboardMonitor.h / .cpp
│
├── shortcuts/                       # Modular hotkey stack (token-friendly split)
│   ├── GlobalHotkeyListener.h / .cpp             # common + registration
│   ├── GlobalHotkeyListener_Win.cpp              # Win native filter
│   ├── GlobalHotkeyListener_X11.cpp              # X11 activity handler
│   └── ShortcutManager.h / .cpp       # configs, enable flags, local QShortcut
│
├── ui/                              # Modular UI (token-friendly split)
│   ├── ClipboardGrabber.h           # class declaration only
│   ├── ClipboardGrabber.cpp         # ~135 lines — ctor / wiring / status
│   │
│   ├── controllers/                 # ClipboardGrabber method implementations
│   │   ├── ClipboardGrabber_Actions.cpp      # setup_actions / services / features
│   │   ├── ClipboardGrabber_Capture.cpp         # start/stop / text / image
│   │   ├── ClipboardGrabber_Capture_Diagram.cpp # live diagram capture
│   │   ├── ClipboardGrabber_Navigation.cpp      # sections helpers
│   │   ├── ClipboardGrabber_Navigation_Subjects.cpp # folders / subjects
│   │   ├── ClipboardGrabber_Headings.cpp        # heading ops
│   │   └── ClipboardGrabber_Dialogs.cpp         # settings / wizards
│   │
│   ├── ClipboardGrabberUI.h / .cpp  # thin panel compositor
│   │
│   ├── panels/                      # self-contained visual blocks
│   │   ├── StatusPanel.h / .cpp
│   │   ├── SubjectPanel.h / .cpp
│   │   ├── CapturePanel.h / .cpp
│   │   ├── HeadingPanel.h / .cpp
│   │   └── ControlsBar.h / .cpp
│   │
│   └── dialogs/
│       ├── HeadingSelectDialog.h / .cpp
│       ├── HeadingSelectDialog_Search.cpp
│       ├── HeadingSelectDialog_Populate.cpp
│       ├── ShortcutsSettingsDialog.h / .cpp
│       └── ExportNoteWizard.h / .cpp
│
└── utils/
    ├── Utils.h / .cpp              # icons, logging, path sanitize, HTML escape
    ├── FileIO.h / .cpp             # atomic QSaveFile read/write (crash-safe)
    ├── CrashGuard.h / .cpp         # terminate + Qt msg handlers, SafeCall
    ├── ConfigPaths.h / .cpp        # ~/GraberNotes/config paths + seed from :/config/*
    └── UiAnimator.h / .cpp         # soft panel collapse + window resize transitions
```

Also under `core/`: `Result.h` (header-only Result&lt;T&gt; / VoidResult).

