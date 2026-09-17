# ADR 0004: One-Shot（一次性）能力支持

## Status

Accepted (2026-09-07)

## Context

### 触发场景

课程参考仓库 `vinceright3/WarriorRPG` commit `d871f47` 引入项目级技能基类 `UWarriorGameplayAbility` 与激活策略枚举 `EWarriorAbilityActivationPolicy`（`OnTriggered` / `OnGiven`）。其中 `OnGiven` 语义为：授予（`OnGiveAbility`）时立即 `TryActivateAbility`，`EndAbility` 时 `ClearAbility` 自毁——即"授予即激活、结束即消耗"的一次性能力。课程首个用例为 `GA_SpawnWeapon`（生成武器并挂背部 Socket，随后自毁）。

### 映射评估中 grill 确认的事实

1. **引擎已内置一次性语义**：`UAbilitySystemComponent::GiveAbilityAndActivateOnce()`（UE5.7 `AbilitySystemComponent_Abilities.cpp:312`）授予并立即激活，设置 `Spec.RemoveAfterActivation = true`；能力结束时由权威端自动 `ClearAbility`（`:1257`）；激活失败立即清除（`:345-349`）。强制约束：必须 Instanced、NetExecutionPolicy 不得为 LocalOnly、仅权威端可调用。
2. **RPGCore 被动授予路径已在复用该 API**：`AddCharacterPassiveAbilities` 对全部初始被动调用 `GiveAbilityAndActivateOnce`。被动永不 `EndAbility`，故 spec 常驻——`RemoveAfterActivation` 对被动是休眠保险。
3. **体系内存在三条授予通道，仅第一条自带移除语义**：① 初始被动/一次性（`GiveAbilityAndActivateOnce`，自带 `RemoveAfterActivation`）；② Eligible/Unlocked 流（普通 `GiveAbility`，无标记，激活推迟到装备时）；③ 装备流（`TryActivateAbility` 激活已有 spec，无标记）。
4. **课程手动 `ClearAbility` 版本无权威端守卫**：客户端预测上下文（`EndAbility` 双端执行）存在误调用风险；引擎路径天然规避。
5. **"技能书/解锁"类需求已有归属**：`Abilities_Status_*` 状态机 + `UnlockOrUpgradeAbility()` 已表达解锁/升级，无需 GA 载体（除非需要读条/打断/演出）。
6. **装备流不可复用**（grill 裁决）：`ServerEquipAbility` 前置条件（spec 已存在且状态为 Unlocked/Equipped）one-shot 不满足；槽位绑定 / 换槽广播 / 法术栏复制全为副作用污染；且装备流的 `TryActivateAbility` 不设置 `RemoveAfterActivation`，结束后 spec 残留。

### 需求定性

一次性能力是跨项目通用 RPG 模式（出生初始化、消耗品、拾取即触发、技能书演出）。按 Warrior `docs/adr/0002` 下沉判据：通用性 ✓、不依赖世界观 ✓、与 GAS 强耦合 ✓、抽象成本可控 ✓、机制与表现分离 ✓。

## Decision

1. **不引入 ActivationPolicy 枚举**（课程的基类枚举与 Lyra 式大枚举均不采用）。RPGCore 保持"Tag 数据驱动分类 + ASC 拥有生命周期"哲学；one-shot 是**授予语境**属性而非能力类型属性，意图编码在入口 API 中。
2. **新增 ASC 授予 API**（与 unlock / equip 流平行，内部统一走 `GiveAbilityAndActivateOnce`）：
   - `AddCharacterOneShotAbilities(const TArray<TSubclassOf<UGameplayAbility>>&)`：startup 批授；
   - `ActivateOneShotAbility(TSubclassOf<UGameplayAbility>, int32 Level = 1)`：运行时单授（物品、世界事件等）。
3. **新增保险丝类 `URPGOneShotAbility : URPGGameplayAbilityBase`**：`EndAbility` 时若 spec 未经 ActivateOnce 路径授予（`RemoveAfterActivation == false`），由权威端兜底 `ClearAbility`。理由：三条授予通道并存，防止误路径（普通 `GiveAbility` / 装备流）下的 spec 残留。
4. **`ARPGCharacterBase` 新增 `StartupOneShotAbilities` 数组**：与 `CharacterAbilities` / `StartupPassiveAbilities` 平行，在 ASC 初始化（引导链）之后由服务端消费。
5. **命名取 "OneShot"，不用 "Instant"**：避免与 GameplayEffect Duration 的 `Instant` 语义混淆。
6. **文档级约定（不加硬校验）**：one-shot 能力不得进入 `DA_AbilityInfo` 注册表与法术栏 UI（`IsPassiveAbility` exact-match 使其天然不可见；违反入口只有配表方）。
7. **边界界定**：快捷栏消耗品 ≠ one-shot。可重复触发的消耗品用普通主动 GA + 冷却 + 物品层扣减建模；one-shot 仅用于"授予即消耗、无第二次触发"的瞬态场景，防止"每次使用重新授予"导致的规格表高频抖动反模式。
8. **技能书两种合法实现**：纯瞬发解锁直接调用 `ASC->UnlockOrUpgradeAbility()`；需要读条 / 可打断 / 获得演出的，以 GA 为载体经 `ActivateOneShotAbility` 授予并触发解锁。

## Consequences

### 正面

- 课程 `OnGiven` 语义完整覆盖，且比课程原版多权威端安全性；
- 与被动模型形成对称 taxonomy：`Passive = 常驻自动激活`、`OneShot = 瞬态自动激活`、主动 = 输入触发常驻；
- API 即文档：授予动作显式声明一次性意图，误用在调用点可见；
- 对 Aura 纯新增：无接口删改，无需同步（`AddCharacterPassiveAbilities` 行为不变）。

### 负面 / 风险

- 消费者误将常驻能力配入 `StartupOneShotAbilities` 会"激活一次即消失"——属配置错误，可在保险丝类输出日志辅助排查；
- 引擎约束需成文：必须 Instanced、NetExecutionPolicy 非 LocalOnly、仅权威端授予、avatar 就绪后授予（依赖引导链时序）。

### 迁移

- **Aura**：无需迁移（纯新增）。
- **Warrior**：随课程 commit `d871f47` 映射实施（本 ADR 先行）；`GA_SpawnWeapon` 落为 `StartupOneShotAbilities` 的首个消费者。

## 相关文档

- Warrior `docs/adr/0002-warrior-code-sink-criteria.md`（下沉判据）
- `docs/adr/0003-playercontroller-genre-neutrality.md`
- Warrior `docs/handoffs/gas-bootstrap-playerstate-asc.md`（引导链衔接点）
- 课程 commit：`d871f476534782e6b9406c9ccfd00152c9815d3c`
- 引擎：`UAbilitySystemComponent::GiveAbilityAndActivateOnce`（`AbilitySystemComponent_Abilities.cpp:312`）
