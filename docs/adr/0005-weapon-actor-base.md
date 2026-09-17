# ADR 0005: 武器 Actor 基类下沉（ARPGWeaponBase）

## Status

Accepted (2026-09-17)

## Context

### 触发场景

课程参考仓库 `vinceright3/WarriorRPG` commit `cff1059`（"Weapon class created"）创建 `AWarriorWeaponBase : AActor`（静态网格根 + 碰撞盒）与空壳子类 `AWarriorHeroWeapon`，并附武器蓝图资产。这是课程武器体系的第一块：后续课程将围绕它展开攻击命中窗口（碰撞盒开关）、Overlap→伤害接线、换装等。

课程的前置链路已在本项目闭环：`GA_SpawnWeapon`（BP，继承 `URPGOneShotAbility`，见 ADR-0004）负责运行时生成武器。

### RPGCore 现状

- 武器基础设施是 **mesh-on-character 范式**：`ICombatInterface::GetWeapon()` 返回 `USkeletalMeshComponent*`；`UCombatComponent::RegisterWeaponMesh(USceneComponent*)` 持有网格引用（Aura 实证：角色自带 `Weapon` 骨骼网格挂 `WeaponSocket`）。
- 该范式无独立生命周期、无每武器碰撞语义——不满足近战 ARPG 的"生成式武器 + 命中窗口"需求。Actor 模型在能力上是严格超集（Actor 内可只放网格，网格长不出生命周期与碰撞）。

### 下沉判据（Warrior ADR-0002 过检）

通用性 ✓（近战 ARPG 通用基础设施）｜不依赖世界观 ✓｜抽象成本可控 ✓（单小类）｜机制表现分离 ✓（基类=机制，斧子外观/数据=宿主项目）。与 CombatComponent / VitalityComponent 同层宿主于 GameplayMechanics。

## Decision

1. **下沉 `ARPGWeaponBase : AActor`** 至 `Source/RPGCore/Public/GameplayMechanics/Core/Weapon/`：
   - 静态网格作根（`UStaticMeshComponent`，课程原样）：Warrior 需求为静态网格武器；若未来出现骨骼网格武器需求，再演进根结构（改 `USceneComponent` 根 + 子网格），本 ADR 不预设；
   - `UBoxComponent` 碰撞盒（Extent 20，默认 `NoCollision`）+ 网格/碰撞盒 getter；
   - 本次**只沉 shell**：不附带命中窗口开关语义、不附带生成/挂载逻辑（均属后续课程 commit 的映射范畴）。
2. **不动 `ICombatInterface`**（Q3，见下节"待后续触发"）。
3. **不建项目层空壳 C++ 子类**：宿主项目 BP（如 Warrior 的 `BP_HeroAxe`）直接继承 `ARPGWeaponBase`；首个真实项目级需求出现时再建子类。
4. **对 Aura 纯 additive**：不修改任何现有接口与类，Aura 无需同步。

## 待后续触发：武器 Actor 契约设计（Q3 记录）

### 本次决策

`ICombatInterface::GetWeapon()` 返回 `USkeletalMeshComponent*`，Actor 武器不满足该网格类型契约。**本次不动接口**：Warrior 侧直接持有武器 Actor 引用（GA / 角色层自行管理）。理由：现在就设计新契约是猜测——真正的需求形状要等"Overlap→伤害"链路的课程 commit 到来才可见。

### 触发条件

课程参考仓库后续出现以下任一类 commit 时，回到本 ADR 执行契约设计：
- 攻击命中窗口（AnimNotify / NotifyState 开关武器碰撞盒）；
- 武器碰撞盒 Overlap 事件处理 / 命中目标收集；
- 近战伤害发送（经 GAS Event 或 DamageGameplayAbility）。

（参见 Warrior `docs/agents/commit-driven-workflow.md` 中早先预判的 `URPGMeleeHitWindowNotify` 下沉候选。）

### 届时设计空间（预设选项，非定案）

| 选项 | 做法 | 代价/风险 |
|---|---|---|
| a. `CombatInterface` 新增 `GetWeaponActor()` | additive BlueprintNativeEvent（带默认实现返回 nullptr），宿主按需覆写 | 零 Aura 影响；接口体逐渐变胖 |
| b. 新独立接口（如 `IRPGWeaponBearer`） | Actor 武器体系自有契约，与 CombatInterface 并列 | 多一个概念；能力查询方需知道查哪个 |
| c. Warrior 项目层局部解决 | 不进 Core 契约 | 第二个宿主项目出现时需重新评估 |

### 硬约束（届时仍生效）

- 变更现有接口的返回类型/签名 = **破坏性改动**，按 ADR-0001 必须同步 Aura 并保证其编译与流程不破坏；
- additive 新增 NativeEvent（带默认实现）对 Aura 安全；
- 契约设计须同时服务两条查询链：① 技能/外部系统查武器 Actor；② 命中窗口内武器反查宿主（Owner）的 ASC 以发送伤害。

## Consequences

### 正面

- 近战"生成式武器"载体进入框架，与 ADR-0004 One-Shot 授予链（`GA_SpawnWeapon`）闭环；
- Aura 零影响；Warrior 蓝图资产可直接基于 Core 基类。

### 负面 / 风险

- 静态网格根写死后，骨骼网格武器项目需演进类结构（已记录演进路径）；
- 武器↔战斗系统契约暂时缺位，Overlap→伤害链路之前 Warrior 侧只能项目内自接（已在"待后续触发"挂号）。

### 迁移

- Aura：无需迁移。
- Warrior：随课程 commit `cff1059` 映射实施（本 ADR 先行）；`BP_HeroAxe` 直接继承 `ARPGWeaponBase`。

## 相关文档

- `docs/adr/0004-one-shot-abilities.md`（生成武器的授予链）
- `docs/prds/WeaponActorBase.md`（本 ADR 的实施规格）
- Warrior `docs/agents/commit-driven-workflow.md`（URPGMeleeHitWindowNotify 预判）
- 课程 commit：`cff105989cff76d97f49d56b971007b5c563ca8a`
