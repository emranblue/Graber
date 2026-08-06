# Clipboard Graber – Scalable Live Diagram System

## What changed

### 1. Dynamic (not buffered) capture
Every clipboard event immediately:
1. Adds the text to the live node list
2. Rebuilds the complete diagram
3. Upserts the live block inside the note file

There is **no deferred write**. The diagram grows in real time.

**Rule enforced in code:**
- 1st copy  → Root node
- 2nd, 3rd… → Sub-nodes of that root

### 2. Rock-solid scalable engine (`DiagramTemplates::buildFromNodes`)
- Supports **any** diagram defined in `config/templates.json`
- Two placeholder styles:
  - **Mermaid** → `{{CONTENT}}`
  - **Custom HTML** → `{{ROOT}}` + `{{SUBNODES}}`
- Adding a new diagram requires **only** editing `templates.json` (no C++ change)

### 3. templates.json tutorial
The `description` field of `config/templates.json` now contains a complete
user-facing tutorial on how to add new diagram types.

## Files modified

```
formatters/DiagramTemplates.h
formatters/DiagramTemplates.cpp
ui/controllers/ClipboardGrabber_Capture_Diagram.cpp
config/templates.json
```

## How to apply

1. Copy the four files above into your existing Graber source tree
   (overwrite the originals).
2. Rebuild:
   ```bash
   make rebuild
   # or
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)
   ```
3. Run. Switch to Diagram mode, choose a diagram type, copy text –
   first copy becomes root, every later copy becomes a sub-node,
   and the note file updates instantly.

## Adding your own diagram (no code change)

Edit `~/GraberNotes/config/templates.json` (or the project `config/templates.json`)
and add:

```json
{
  "key": "my_radial",
  "display_name": "My Radial Diagram",
  "is_diagram": true,
  "body": "```mermaid\nmindmap\n{{CONTENT}}\n```\n"
}
```

Save. Graber reloads templates live. The new entry appears in the
diagram dropdown automatically.
