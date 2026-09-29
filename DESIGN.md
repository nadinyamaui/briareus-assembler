---
name: Briareus for Windows
description: Briareus warm neutral and clay identity adapted to native Windows controls.
colors:
  clay: "#C96442"
  warm-background: "#FAF9F5"
  white-surface: "#FFFFFF"
  dark-ink: "#262525"
typography:
  display:
    fontFamily: "Georgia"
    fontWeight: 400
  body:
    fontFamily: "Segoe UI"
    fontWeight: 400
  detail:
    fontFamily: "Consolas"
    fontWeight: 400
---

# Design System: Briareus for Windows

## Overview

Mode: Operate. Preserve the original iOS application's warm neutral and clay identity, adapted to native Windows desktop controls. The FASM and Win32 implementation combines a quiet canvas, a serif brand title and practical lists and text panes.

This records the built interface. Native Windows rendering supplies button, selection, focus and disabled treatments. The original application remains the authority for feature semantics; [coverage and limitations](docs/PARITY.md) records incomplete parity and outstanding real-server validation.

**Key Characteristics:**

- Warm neutral canvas with a clay wordmark.
- Standard native controls and a persistent status line.
- Three adaptive size classes: three panes, navigation plus detail, or one pane.
- Plain Unicode text, monospaced details and editable JSON for advanced operations.

## Colors

### Primary

**Clay** identifies the Briareus wordmark. Buttons keep their native appearance rather than receiving an accent fill.

### Neutral

**Warm background** covers the main window, labels and status. **White surface** covers edits, lists and combo text areas, including read-only content. **Dark ink** supplies application-painted text. Frontmatter records the actual Win32 colors; selection, disabled and other native states also use system colors.

**The Opaque Surface Rule.** Combo boxes and read-only edits paint opaque backgrounds so refreshed text does not retain stale glyphs.

## Typography

- **Display:** Georgia, regular, requested Win32 font height (-32 logical units), for Briareus only.
- **Body and labels:** Segoe UI, regular, requested height (-17 logical units), for controls, headings, help, composer and status.
- **Detail:** Consolas, regular, requested height (-15 logical units), for the read-only detail pane.

These are CreateFontW requests, not web font sizes. Windows supplies fallback and rasterization. The application uses system DPI awareness; per-monitor scaling is not implemented.

## Layout

The resizable desktop window has a minimum outer size of 480 by 680. Layout responds to the current client area, not a device model. Measurements below are native client-coordinate units at system DPI.

- At 1100 client units or wider, projects (224 wide), list (264 wide) and the growing detail pane are visible together.
- From 800 through 1099 client units, a 240-wide navigation pane switches between projects and the item list while the detail pane remains visible.
- Below 800 client units, one pane fills the available width. Projects, List and Detail buttons provide explicit navigation. Selecting a project opens its list; selecting an item opens detail.

Pairing fields shrink to the available width. Refresh, Disconnect and Revoke token remain reachable above the content. The compact navigation row appears only below the three-pane size. Panel switching and resizing reuse the same controls and preserve selection, transcript, unsent draft and edited API arguments.

Detail and lists stretch vertically. The composer follows the lower edge. Below 520 units of detail width, message actions wrap into two rows; API actions remain a pair. Status stays at the lower edge. Long text scrolls within its native control. Theme remains light, and DPI awareness remains system-level; per-monitor DPI scaling is not implemented.

## Elevation & Depth

There are no application-defined shadows or layered cards. Warm background and white content surfaces establish separation. Native Windows rendering supplies control borders, button relief, dropdowns and dialogs.

## Shapes

Rectangular native lists, edits, combos and buttons define the interface. There is no custom radius scale, chip system or card silhouette; system rendering determines border and button geometry.

## Components

### Buttons

Connect, Refresh, Disconnect, Revoke device token, Send message, New conversation and Actions / API are native text buttons. Advanced mode replaces composer actions with Execute and Back to message. Focus, pressed and disabled appearances belong to Windows. Disconnect, revoke and advanced write operations use native confirmation dialogs.

### Inputs / Fields

Pairing fields have cue text, and the device token is masked. Search filters loaded row data. The multiline composer accepts a message or a task for a new conversation. In advanced mode the same field becomes an editable JSON argument editor; Back to message restores the saved draft.

Read-only access makes the ordinary composer read-only and disables message/new-conversation buttons; its label directs users to Actions / API for read operations. The JSON editor remains editable for those reads. Sending requires a selected conversation; a new conversation requires a selected project. Board views label the composer "Task for a new conversation" and disable Send message.

### Navigation

The project list selects the repository. The view dropdown offers Conversations, Pull requests, Issues and API tools. The middle list selects a conversation or board row. There are no graphical board cards or faceted filters.

Both the view and operation dropdowns explicitly use the standard **unthemed native combo box renderer**, through SetWindowTheme with empty strings. This is the actual repaint treatment, not a custom-styled combo.

### Detail and API tools

The read-only multiline detail edit presents Unicode conversation events, textual board descriptions, formatted JSON and monospaced patches. Rich Markdown, syntax highlighting and navigable links are not implemented.

Actions / API exposes server-advertised operations through a dropdown, editable JSON templates and Execute. Available operations respect read/manage permissions. Advanced actions use this interface rather than dedicated graphical controls for every iOS feature.

### Status and request states

The persistent status line shows guidance, pending requests, permissions and errors. Network requests run on a worker thread; request-triggering controls disable while busy. An uncertain write outcome blocks further writes until explicit refresh and verification. Selected conversations poll every seven seconds in the active conversation view; polling pauses after errors and when the window is inactive.

## Do's and Don'ts

### Do:

- **Do** preserve the Briareus name, warm canvas and clay wordmark.
- **Do** retain native keyboard focus, selection and disabled states, plus token masking.
- **Do** keep combo and read-only text surfaces opaque during repaint.
- **Do** consult docs/PARITY.md before making feature-coverage claims.

### Don't:

- **Don't** describe the current interface as complete iOS parity or validated against a real Briareus server.
- **Don't** describe the unthemed native combos as custom or themed components.
- **Don't** imply rich messages, graphical board cards, dark mode or per-monitor DPI support are already built.
- **Don't** translate native controls into invented CSS selectors, web breakpoints or component snippets.
