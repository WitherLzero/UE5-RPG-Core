# PRD: RPGCore Module → Plugin 转换

## Problem Statement

RPGCore 目前以 UE5 Runtime 模块的形式分发，位于 `Source/RPGCore/`，作为 git 子模块嵌入宿主项目。它关联的编辑器代码（`RPGCoreEditor`）则因为 Editor 模块类型无法放入同一个 Runtime 模块中，只能作为独立目录存在于宿主项目的 `Source/RPGCoreEditor/`。这导致：

1. **分发不对称**：开发者 clone RPGCore 子模块后，只得到运行时代码；编辑器工具需要手动从宿主项目分离再带到下一个项目。
2. **编辑工具碎片化**：RPGCore Runtime 模块的 `EditorTools/` 目录中已有间接引用 `AssetRegistry` 的代码，形成模糊的依赖边界。
3. **蓝图资产不随代码走**：Editor Utility Widget（EUW_GameplayTagBrowser、WBP_TagChip）等依赖 C++ 类的蓝图资产位于项目 Content/ 中，无法随 RPGCore 自动携带。

## Solution

将 RPGCore 从 UE5 Module 转换为 UE5 Plugin。Plugin 包含两个子模块：

- **RPGCore**（Runtime 模块）— 现有 RPGFramework + GameplayMechanics 代码
- **RPGCoreEditor**（Editor 模块）— 现有 RPGCoreEditor 编辑器扩展 + 从 Runtime 迁移过来的 EditorTools

Plugin 自带 `Content/` 目录容纳所有相关蓝图资产，实现「一个 plugin 目录 drop 进项目即可使用」的自包含分发。

## User Stories

1. As a **project developer integrating RPGCore**, I want to **clone the repo as a plugin into `Plugins/`** instead of `Source/`, so that **setup steps减少**（不再需要手动修改 `.uproject` Modules 和 Target.cs）。
2. As a **project developer integrating RPGCore**, I want **RPGCoreEditor 随插件自动可用**，无需从宿主项目管理其代码，所以**编辑器工具按预期工作**。
3. As a **project developer using the GameplayTag Browser**, I want **EUW_GameplayTagBrowser 和 WBP_TagChip 等资产在插件中直接存在**，无需从项目 Content/ 手动复制。
4. As a **project developer upgrading RPGCore**, I want **现有的项目代码（Aura 模块）无需重写**，只需调整少量集成配置即可继续使用。
5. As a **framework maintainer**, I want **RPGCore 和 RPGCoreEditor 代码在同一个仓库中**，确保版本同步、CI 统一、issue 追踪一致。
6. As a **framework maintainer**, I want **`EditorTools/` 中 Editor-only 的代码移到 RPGCoreEditor 模块**，避免 Runtime 模块携带 editor 依赖。
7. As a **framework maintainer**, I want **README 集成指南反映新的 Plugin 结构**，让新用户能按正确步骤集成。
8. As a **project developer verifying the migration**, I want **Game target + Editor target 编译通过**，无链接错误。
9. As a **project developer verifying the migration**, I want **编辑器启动后 RPGCore 插件被正确加载**，模块日志无报错。
10. As a **project developer verifying the migration**, I want **进入 PIE 后 GameplayTag 初始化正常运行**，原 GAS 功能不受影响。
11. As a **project developer verifying the migration**, I want **EUW_GameplayTagBrowser 能正常打开**，Tag picker 和 Reference Viewer 功能与迁移前一致。

## Implementation Decisions

### 架构决策

- **Plugin 结构**：单一 Plugin 内含两个 UE5 Module（RPGCore Runtime + RPGCoreEditor Editor），使用标准 UE5 Plugin 目录布局。
- **分发方式**：保持 git 子模块方式，但目录从 `Source/RPGCore` 迁移到 `Plugins/RPGCore`。
- **仓库结构**：在 RPGCore git repo 根目录放置 `RPGCore.uplugin` 描述文件，`Source/` 目录下分两个子模块目录。
- **Content 资产**：所有相关的 Editor Utility Widget 和 Widget Blueprint 资产移入 Plugin 的 `Content/` 目录。
- **EditorTools 搬迁**：`RPGEditorFunctionLibrary` 和 `RPGEditorListItemData` 从 RPGCore 移入 RPGCoreEditor 模块，API 宏从 `RPGCORE_API` 更新为 `RPGCOREEDITOR_API`。
- **依赖清理**：RPGCore.Build.cs 移除 `AssetRegistry`（已不再需要）。

### Aura 项目侧变更

- `.gitmodules`：`path = Source/RPGCore` → `Plugins/RPGCore`
- `Aura.uproject`：从 `Modules` 数组移除 RPGCore/RPGCoreEditor；在 `Plugins` 数组添加 RPGCore（Editor target allow list）
- `Aura.Target.cs`：`ExtraModuleNames` 移除 `RPGCore`
- `AuraEditor.Target.cs`：`ExtraModuleNames` 移除 `RPGCore`、`RPGCoreEditor`
- `Aura.Build.cs`：保持 `PublicDependencyModuleNames.Add("RPGCore")` 不变
- 删除 `Source/RPGCoreEditor/` 旧目录

### 不做的决策

- Aura.h 中与 RPGCore.h 重复的 collision channel 宏定义保持现状（不改 include 路径）
- 不创建向后兼容分支（仅 Aura 一个消费者）
- 不改变 `FRPGCoreModule` 的模块注册逻辑或 GameplayTag 初始化逻辑

## Testing Decisions

这是一次结构性迁移，测试重点在于功能和行为的**一致性**：

| Seam | 验证方式 | 预期结果 |
|------|----------|----------|
| Compilation (Game target) | 编译 Development Game | 编译成功，无链接错误 |
| Compilation (Editor target) | 编译 Development Editor | 编译成功，无链接错误 |
| Plugin discovery | 启动编辑器，检查 Modules 日志 | RPGCore + RPGCoreEditor 正确加载 |
| GameplayTag init | 进入 PIE，检查 GameplayTag 管理器 | FRPGGameplayTags 正常初始化 |
| Editor tooling | 打开 EUW_GameplayTagBrowser | Tag picker + Reference Viewer 正常工作 |
| Content portability | 检查 Plugin Content/ 资产 | 资产可正常引用、实例化，无缺失引用 |
| RPGCore API | 检查现有 GAS 功能（技能、属性、AI Tag 桥接） | 功能与迁移前行为一致 |

不需要为迁移本身编写自动化测试。迁移后可用现有项目的手动测试流程验证。

## Out of Scope

- 不改变 RPGCore 的 API 接口或 C++ 类名
- 不修改 RPGCore 的功能代码（GameplayMechanics、RPGFramework）
- 不添加新的编辑器功能
- 不处理引擎级 Plugin 分发（保持 Project Plugin 形式）
- 不在 RPGCore plugin 中引入新的内容资产（仅迁移现有资产）

## Further Notes

- 迁移应在 RPGCore git repo 的 `main` 分支上直接进行（仅 Aura 一个消费者，不需要兼容分支）
- 迁移完成后需更新 RPGCore README.md 中的集成指南
- 参考 `ADR 0002: RPGCore Module → Plugin 转换`（位于 `docs/adr/0002-module-to-plugin-conversion.md`）
