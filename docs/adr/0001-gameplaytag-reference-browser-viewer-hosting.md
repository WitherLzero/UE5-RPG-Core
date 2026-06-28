# ADR 0001: GameplayTag Reference Browser — Reference Viewer Hosting

## Status

Accepted

## Context

The GameplayTag Reference Browser is an editor utility for RPGCore/Aura that lets users multi-select GameplayTags and inspect which assets reference those tags. A key design question is how to display the reference graph:

- **Option A (Embedded Slate widget):** Host `SReferenceViewer` directly inside the Editor Utility Widget (EUW) via a custom Slate-to-UMG bridge widget.
- **Option B (Standalone Reference Viewer tab):** Open the engine's native Reference Viewer in a separate tab by broadcasting `FEditorDelegates::OnOpenReferenceViewer` with multiple `FAssetIdentifier` roots.

## Decision

Use **Option B** for the first implementation.

## Consequences

### Positive

- Lower implementation cost: no custom Slate/UMG bridge, no duplicate Zoom/History/Filter UI.
- Reuses the full native Reference Viewer feature set immediately.
- Validates the core value proposition — multi-tag batch reference queries — before investing in deeper UI integration.
- The EUW layout can be designed so the bottom section is later replaceable with an embedded viewer host.

### Negative

- Reference graph opens in a separate tab, not inside the same window as the Tag picker.
- Future transition to embedded mode requires replacing the output layer and adding a Slate host widget.

## Future Work

If embedded hosting proves valuable, evaluate creating a custom `UWidget` subclass that wraps `SReferenceViewer` and exposes it to UMG. This should only be attempted after the standalone workflow is validated.
