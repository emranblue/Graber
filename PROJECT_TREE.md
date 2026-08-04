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
│   ├── MarkdownDocumentFormatter.h / .cpp
│   ├── MarkdownUtils.h / .cpp
│   └── DiagramTemplates.h / .cpp
│
├── monitors/
│   └── ClipboardMonitor.h / .cpp
│
├── shortcuts/                       # Modular hotkey stack (token-friendly split)
│   ├── GlobalHotkeyListener.h / .cpp  # OS global grab (Win / X11)
│   └── ShortcutManager.h / .cpp       # configs, enable flags, local QShortcut
│
├── ui/                              # Modular UI (token-friendly split)
│   ├── ClipboardGrabber.h           # class declaration only
│   ├── ClipboardGrabber.cpp         # ~135 lines — ctor / wiring / status
│   │
│   ├── controllers/                 # ClipboardGrabber method implementations
│   │   ├── ClipboardGrabber_Actions.cpp      # setup_actions / services / features
│   │   ├── ClipboardGrabber_Capture.cpp      # start/stop / text / image / diagram
│   │   ├── ClipboardGrabber_Navigation.cpp   # folders / subjects / sections
│   │   └── ClipboardGrabber_Headings.cpp     # heading ops / settings / wizards
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
│       ├── ShortcutsSettingsDialog.h / .cpp
│       └── ExportNoteWizard.h / .cpp
│
└── utils/
    └── Utils.h / .cpp
```
