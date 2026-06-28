# ADR 0002: RPGCore Module → Plugin 转换

RPGCore 从一个独立 UE5 模块（`Source/RPGCore`，Runtime 类型）和一个位于宿主项目中的伴侣模块（`Source/RPGCoreEditor`，Editor 类型）的方案，改为一个包含两个子模块的自包含插件：一个 RPGCore Runtime 模块和一个 RPGCoreEditor Editor 模块，作为一个单一 git 仓库中的 UE5 插件发布。

## Status

Accepted

## Context

RPGCore 最初设计为标准的 UE5 模块，位于 `Source/RPGCore`，作为一个 git 子模块嵌入宿主项目。其编辑器代码（`RPGCoreEditor`）不得不作为宿主项目中的一个独立 *Editor* 类型模块存在，因为 UE5 不允许 Runtime 模块直接在内部编译时依赖 `UnrealEd`。这导致两个问题：

1. **分发不对称**：RPGCore 仅作为子模块携带运行时代码，但编辑器代码必须由宿主项目手动管理。
2. **编辑工具碎片化**：`RPGCore`（Runtime 模块）的 `EditorTools/` 目录中包含间接引用 `AssetRegistry`（编辑器相关）的代码，形成模棱两可的依赖形态。

将 RPGCore 发布为 UE5 插件，可将运行时模块和编辑器模块合并到同一个可重复消费的单元中，并通过 `Content/` 目录携带蓝图资产。

## Considered Options

- **Option A — 仅将 RPGCoreEditor 迁移为插件**：保持 RPGCore 作为子模块不变。被否决，因为它延续了将某个模块分散在两个仓库中的模式，同时插件实际上更适合承载多个模块。
- **Option B — 将两个模块都保留在宿主项目中**：当前现状。被否决，因为分发姿态脆弱（编辑器模块必须手动迁移到每个新宿主项目中）。
- **Option C — 将两个模块都合并为一个单一模块，运行时通过 `WITH_EDITOR` 保护编辑器路径**：技术上可行，但构建摩擦增加（编辑器头文件泄漏到运行时包含路径中），且 IDE 中的关注点分离更差。

## Consequences

- 宿主项目必须将子模块重新定位到 `Plugins/RPGCore`，而不是 `Source/RPGCore`。
- `AuraEditor.Target.cs` 和 `Aura.Target.cs` 中的 `ExtraModuleNames` 条目必须移除，因为 UE5 插件会自动发现模块。
- `Aura.uproject` 的 `Modules` 数组中的条目必须移除，并替换为 `Plugins` 数组中的条目。
- 所有内容资产（Editor Utility Widget、WBP_TagChip）必须迁移到插件自己的 `Content/` 目录中。
- 现有的 `RPGEditorFunctionLibrary` 和 `RPGEditorListItemData` 类必须从 RPGCore 迁移到 RPGCoreEditor 模块，并更新其 API 导出宏。
- 集成 README 必须更新，以反映新的子模块路径和基于插件的集成指南。
