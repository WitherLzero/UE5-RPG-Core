# PRD: One-Shot（一次性）能力支持

## 背景

课程参考仓库 `vinceright3/WarriorRPG` commit `d871f47` 以 `EWarriorAbilityActivationPolicy::OnGiven` 表达"授予即激活、结束即消耗"的一次性能力（首个用例：`GA_SpawnWeapon` 生成武器挂背部 Socket 后自毁）。映射评估确认引擎 `GiveAbilityAndActivateOnce` 已内置该语义，RPGCore 需将其升级为框架级一等公民。设计决策已全部定案于 `docs/adr/0004-one-shot-abilities.md`，本 PRD 为其实施规格。

## 方案

与被动模型对称的 taxonomy：**Passive = 常驻自动激活 / OneShot = 瞬态自动激活**。不引入 ActivationPolicy 枚举；一次性意图编码在授予 API（授予语境属性）。全部改动为 additive，Aura 无需同步。

## User Stories

1. 作为 **宿主项目开发者**，我想把出生初始化类能力（如生成武器）配进 `StartupOneShotAbilities` 数组，使角色被 Possess 后自动授予、激活、自毁，无需手写授予代码。
2. 作为 **物品系统开发者**，我想通过 `ActivateOneShotAbility(Class)` 在运行时（如使用技能书）授予一次性能力，并依赖框架保证结束后 spec 无残留。
3. 作为 **框架维护者**，我想一次性语义在任意授予路径下都成立（保险丝兜底），并在误用时得到日志提示。

## Implementation Decisions

### ① 新增 `URPGOneShotAbility`（保险丝类）

- 新文件：
  - `Source/RPGCore/Public/RPGFramework/GAS/Abilities/RPGOneShotAbility.h`
  - `Source/RPGCore/Private/RPGFramework/GAS/Abilities/RPGOneShotAbility.cpp`
- 继承 `URPGGameplayAbilityBase`（与 `RPGFramework/GAS/Abilities/RPGGameplayAbilityBase.h` 同目录）。
- 覆写 `EndAbility`：`Super` 之后，**仅权威端**检查 `GetCurrentAbilitySpec()->RemoveAfterActivation`；若为 false（说明走了普通 `GiveAbility` 或装备流等无移除语义的授予路径），输出 `Warning` 日志（含能力名与正确授予方式提示）并 `AbilitySystemComponent->ClearAbility(Handle)`。
- 经 `GiveAbilityAndActivateOnce` 授予时（RemoveAfterActivation == true）保险丝不动作，由引擎清理——双保险结构。

### ② `URPGAbilitySystemComponent` 新增两个授予 API

- 头文件（public 区，`AddCharacterPassiveAbilities` 附近）声明：

```cpp
void AddCharacterOneShotAbilities(const TArray<TSubclassOf<UGameplayAbility>>& OneShotAbilities);
FGameplayAbilitySpecHandle ActivateOneShotAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level = 1);
```

- 实现（`RPGAbilitySystemComponent.cpp`）：
  - 两者开头均做 `IsOwnerActorAuthoritative()` 守卫（引擎 API 权威端限定，防御客户端误调）；
  - 空类 / 空数组直接返回；
  - 内部统一构造 `FGameplayAbilitySpec(AbilityClass, Level)` 并调用引擎 `GiveAbilityAndActivateOnce(Spec)`（UE5.7 签名为非 const 引用）；
  - 引擎已负责 InstancingPolicy / NetExecutionPolicy 校验与激活失败清理，不重复实现。

### ③ `ARPGCharacterBase` 接入引导链

- 头文件（`RPGFramework/Character/RPGCharacterBase.h`，`StartupPassiveAbilities` 旁）新增：

```cpp
UPROPERTY(EditAnywhere, Category = "Abilities")
TArray<TSubclassOf<UGameplayAbility>> StartupOneShotAbilities;
```

- `InitAbilityActorInfo()` 默认实现中，ASC init 之后追加（**仅权威端**，因授予是服务端行为）：

```cpp
if (HasAuthority() && !StartupOneShotAbilities.IsEmpty())
{
    if (URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(AbilitySystemComponent))
    {
        RPGASC->AddCharacterOneShotAbilities(StartupOneShotAbilities);
    }
}
```

- `.cpp` 需 `#include "RPGFramework/GAS/RPGAbilitySystemComponent.h"`。
- 不动 `InitDefaultAttributes()` / `AddCharacterAbilities()`（仍为空壳，超出本 PRD 范围）。

## 验收标准

- [ ] WarriorEditor Win64 Development 编译通过（exit 0）。
- [ ] 新增/修改仅限上述 6 个文件；不触碰 Aura 与 `AddCharacterPassiveAbilities` 现有行为。
- [ ] 代码风格与 RPGCore 现有文件一致（Tab 缩进、`RPGCORE_API`、UE_LOG 用 `LogTemp`）。

## 相关文档

- `docs/adr/0004-one-shot-abilities.md`（设计决策）
- 引擎参考：`UAbilitySystemComponent::GiveAbilityAndActivateOnce`（`AbilitySystemComponent_Abilities.cpp:312`）
