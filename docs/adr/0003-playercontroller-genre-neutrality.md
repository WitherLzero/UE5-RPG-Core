# ADR 0003: ARPGPlayerController 视角中立化 — 光标逻辑迁出基类

## Status

Accepted

## Context

ARPGPlayerController 从 Aura(俯视角 RPG)提取时携带了两块视角相关逻辑:

- `CursorTrace()` — 光标下敌兵检测 + 高亮(30Hz 节流)
- `UpdateMouse()` — 光标显隐 / InputMode 设置(代码内原有 "Top-Down 专属" TODO)

Warrior(第三人称近战 ARPG)作为第二个宿主接入时,这两块均不适用,调用点已在 RPGCore 内注释(即当前为死代码)。继续留在基类带来三个问题:

1. 按视角类型在通用基类堆积逻辑,违背插件的通用性定位;
2. 业界无"按视角分子类化 PlayerController"的受尊重先例 —— Lyra 模式是薄 PC + Pawn 组件 + 配置资产,差异通过组件与配置表达;
3. 框架内已有正确先例:`ARPGTargetActor_Indicator` 将光标轮询放在机制所有者(GAS TargetActor)内,PC 仅被临时借用。

## Decision

1. **基类视角中立**:ARPGPlayerController 只保留跨视角通用职责(输入链、ASC、伤害文字池)+ 光标可见性/InputMode 的**配置化**:
   - 5 个 `EditDefaultsOnly` 属性(`bShowCursor`、`MouseCursorType`、`bUseGameAndUIMode`、`MouseLockMode`、`bHideCursorDuringCapture`),宿主在 BP Class Defaults 或构造函数中设定默认值;
   - BeginPlay 内联应用一次,**不提供**动态切换方法 —— 需要动态显隐光标的技能逻辑自行调用引擎 API(机制随所有者走)。
2. **光标驱动的高亮机制组件化**:新增 `UCursorHighlightComponent`(挂 Pawn),承载节流追踪、`Player_Block_CursorTrace` Tag 门控、`LastActor/ThisActor` 高亮状态机与 `GetHoveredEnemy()` 查询。
3. **不按视角建立 PC 子类**:top-down 与 third-person 的差异通过"挂不挂组件 + 属性默认值"表达。
4. `Player_Block_CursorTrace` GameplayTag 保留在 RPGCore(删除 Native Tag 会破坏已有引用)。
5. `STAT_CursorTrace` 从 `RPGCoreStats.h` 移除,改为组件 .cpp 内局部声明。

## Consequences

### 破坏性面(删除的 public API)

- `ARPGPlayerController::GetCursorHit()` / `HitEnemyActor()` — 由 `UCursorHighlightComponent::GetHoveredEnemy()` 取代(语义等价)。
- `STAT_CursorTrace` — 由组件内局部 stat `STAT_CursorHighlight` 取代。

### Aura 迁移路径

- 现在:**零改动**。Aura Source 无引用,且调用点此前已注释(功能未运行),编译不受影响。
- 复活时:在 Pawn(如 AuraCharacter)构造函数挂载 `UCursorHighlightComponent`;光标默认值(`bShowCursor=true`、`bUseGameAndUIMode=true`)与 Aura 原行为一致。
- 建议在 Aura 编辑器内全局搜索一次蓝图对 `GetCursorHit` / `HitEnemyActor` 的引用(Source 层 grep 无法覆盖蓝图资产)。

### 正面

- 基类对任何视角类型无假设;新宿主(如第三人称近战 ARPG)零负担继承。
- 高亮机制可组合:任何宿主按需在 Pawn 上挂载,不继承即不携带。

### 负面

- 高亮机制的当前唯一消费者是(停摆的)Aura —— 组件属于为单一休眠消费者沉淀的库存。成本是一个小自包含类,可接受;若长期无第二个消费者,可在后续清理中重新评估。

## 相关文档

- `CONTEXT.md`
- `docs/adr/0002-module-to-plugin-conversion.md`
