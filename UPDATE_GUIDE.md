# Graber — Update & Architecture Guide

This document serves as a developer guide for understanding the project architecture, extending the application with new features, services, buttons, options, and wizards, maintaining `PROJECT_TREE.md`, and managing Git workflow.

---

## 1. System Architecture Overview

Graber is designed adhering strictly to **SOLID principles**, **Clean Architecture**, and **Dependency Inversion**:

```
graber/
├── core/         # Data structures, domain models, enums (Types.h)
├── interfaces/   # Abstract contracts & interfaces (INoteService, IServiceRegistry, IAction, IWizardFeature, etc.)
├── services/     # Domain service implementations, Service Locator, Action Registry, & Feature Manager
├── repositories/ # Storage & persistence layer (NoteRepository, IniSectionRepository)
├── formatters/   # Markdown parsing, HTML normalization, and TOC generation engines
├── monitors/     # Operating system clipboard and primary selection poller
├── shortcuts/    # Hotkey registration and shortcut settings manager
├── ui/           # Qt UI controllers, views, dialogs, and feature wizards
└── utils/        # Vector icons, font loaders, styling helpers, and debug loggers
```

---

## 2. Abstraction & Extension Layers

### A. Service Layer & Service Locator (`INoteService` / `ServiceRegistry`)
All application services are registered in the global `ServiceRegistry` singleton:
- Interface: `INoteService` ([interfaces/INoteService.h](file:///home/emran/Desktop/extra/graber/interfaces/INoteService.h))
- Implementation: `NoteService` ([services/NoteService.h](file:///home/emran/Desktop/extra/graber/services/NoteService.h))
- Registry: `ServiceRegistry` ([services/ServiceRegistry.h](file:///home/emran/Desktop/extra/graber/services/ServiceRegistry.h))

**How to register or retrieve a service:**
```cpp
// Registering a service:
auto noteSvc = std::make_shared<NoteService>();
ServiceRegistry::instance().registerService<INoteService>(noteSvc);

// Retrieving a service anywhere in the codebase:
auto noteSvc = ServiceRegistry::instance().getService<INoteService>();
```

---

### B. Command / Action System (`IAction` / `ActionRegistry`)
Actions decouple user intent (button clicks, hotkeys, context menu triggers) from UI implementation.
- Interface: `IAction` / `FunctionalAction` ([interfaces/IAction.h](file:///home/emran/Desktop/extra/graber/interfaces/IAction.h))
- Registry: `ActionRegistry` ([services/ActionRegistry.h](file:///home/emran/Desktop/extra/graber/services/ActionRegistry.h))

**How to add a new action & bind it to a button:**
1. Register the action in `setup_actions()` inside `ClipboardGrabber.cpp`:
   ```cpp
   ActionRegistry::instance().registerFunctionalAction(
       "my_action_id",                     // Unique ID
       "আমার অ্যাকশন (My Action)",        // Display Title
       "আমার ফিচারের বিবরণ",             // Description
       "CategoryName",                    // Category
       QKeySequence("Ctrl+Shift+X"),     // Default Shortcut
       [this](const QVariantMap &args) {   // Execution Handler
           // Your code logic here
       },
       [this]() {                         // Enabled Predicate (Optional)
           return true; 
       }
   );
   ```
2. Bind a button in UI to the action:
   ```cpp
   ActionRegistry::instance().bindButton(ui_.my_button, "my_action_id");
   ```
   *Note: `ActionRegistry` automatically manages button enabled/disabled states and key sequence shortcuts!*

---

### C. Extensible Wizard & Feature Architecture (`IWizardFeature` / `FeatureManager`)
Wizards are self-contained interactive tools (dialogs, export managers, batch processors) added without touching core UI layout logic.
- Interface: `IWizardFeature` ([interfaces/IWizardFeature.h](file:///home/emran/Desktop/extra/graber/interfaces/IWizardFeature.h))
- Manager: `FeatureManager` ([services/FeatureManager.h](file:///home/emran/Desktop/extra/graber/services/FeatureManager.h))
- Example Implementation: `ExportNoteWizard` ([ui/ExportNoteWizard.h](file:///home/emran/Desktop/extra/graber/ui/ExportNoteWizard.h))

**How to create & register a new Wizard:**
1. Create a class implementing `IWizardFeature` (e.g. `MyCustomWizard.h` & `.cpp`).
2. Implement `id()`, `displayName()`, `description()`, `iconName()`, `category()`, and `executeWizard(...)`.
3. Register the wizard in `setup_features()` inside `ClipboardGrabber.cpp`:
   ```cpp
   FeatureManager::instance().registerFeature(std::make_shared<MyCustomWizard>(this));
   ```
   *The wizard will automatically appear in the "উইজার্ড ও টুলস" (Wizards & Tools) menu!*

---

## 3. Step-by-Step Workflow for Future Updates

Whenever adding a new file, service, feature, or dialog to `graber`:

### Step 1: Create Source Files
Place new files in the appropriate directory:
- Contracts/Interfaces -> `interfaces/`
- Domain Services & Registries -> `services/`
- Persistence/Repositories -> `repositories/`
- Custom UI/Wizards -> `ui/`
- Core Data Models -> `core/`

### Step 2: Update `CMakeLists.txt`
Add the new `.h` and `.cpp` files under the target section in [CMakeLists.txt](file:///home/emran/Desktop/extra/graber/CMakeLists.txt).

### Step 3: Update `PROJECT_TREE.md`
Update [PROJECT_TREE.md](file:///home/emran/Desktop/extra/graber/PROJECT_TREE.md) to reflect the new directory structure and files.

### Step 4: Build & Test
Use CMake or Makefile wrapper:
```bash
make rebuild
```

---

## 4. Git Version Control & Commit Conventions

When making git commits for project updates:

1. **Check Status & Changes**:
   ```bash
   git status
   ```
2. **Stage Modified & New Files**:
   ```bash
   git add .
   ```
3. **Write Meaningful Commit Messages**:
   Use structured commit message formats:
   - `feat:` for new features, buttons, or wizards
   - `refactor:` for structural or architectural abstractions
   - `fix:` for bug fixes
   - `docs:` for documentation or project tree updates

   *Examples:*
   ```bash
   git commit -m "feat(wizard): add new section batch manager wizard"
   git commit -m "docs(tree): update PROJECT_TREE.md for new services"
   ```
