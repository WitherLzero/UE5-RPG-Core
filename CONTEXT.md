# RPGCore - Shared RPG Systems

This context covers the RPGCore shared plugin, a collection of reusable Unreal Engine 5 RPG systems extracted from Aura for use across multiple projects.

## Domain

- Unreal Engine 5
- Shared RPG gameplay systems (GAS helpers, attribute utilities, common ability patterns, etc.)
- Editor tooling for RPG content governance (e.g. GameplayTag analysis, asset dependency inspection)
- Engine-level or framework-level code, not game-specific logic

## Reuse convention

RPGCore is maintained as an independent Git repository and included in consuming projects as a Git submodule. It must remain free of game-specific concepts from any single project.

## Submodule usage

- Repository: `https://github.com/WitherLzero/UE5-RPG-Core.git`
- Included in Aura at `Plugins/RPGCore` (since ADR 0002)

## Responsibilities

- Provide reusable RPG building blocks
- Provide editor utilities that help manage RPG content across projects
- Avoid coupling to Aura-specific assets, gameplay, or content
- Document breaking changes and migration notes in `docs/adr/`

## Documentation structure

```
docs/
├── adr/           # Architecture Decision Records (breaking changes, migration)
├── editor-tools/  # Editor utility documentation & implementation reports
└── handoffs/      # Agent handoff documents for session continuity
```

### Agent handoff convention

Handoff documents MUST be saved to `docs/handoffs/`, never to OS temp directories. This ensures handoffs survive machine restarts and remain discoverable by other agents. The filename should follow the pattern `{topic}-handoff.md`.

## Related contexts

- `../../CONTEXT.md` - Aura business project consuming this plugin

## Glossary

| Term | Meaning |
|---|---|
| **GameplayTag Reference Browser** | An editor utility for inspecting GameplayTags and visualizing their asset references across a project. |
| **Tag Chip** | A small removable UI element that displays a selected GameplayTag in the GameplayTag Reference Browser. |
| **Editor Utility Widget (EUW)** | A UMG-based editor tool authored in Blueprint and driven by C++ helper functions. |
