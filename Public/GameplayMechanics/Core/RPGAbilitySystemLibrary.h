// Copyright rynnli

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RPGAbilitySystemLibrary.generated.h"

class UNiagaraSystem;
struct FDamageEffectParams;
struct FGameplayEffectSpecHandle;
class ARPGProjectile;
class UAbilityInfo;
struct FTaggedMontage;
class UGameplayEffect;
struct FGameplayEffectContextHandle;
class UAbilitySystemComponent;
class UAttributeMenuWidgetController;
/**
 * 
 */
UCLASS()
class RPGCORE_API URPGAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	/*
	 * Ability Info
	 */

	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|AbilityInfo")
	static UAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);

	/*
	 * Gameplay Mechanics
	 */

	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayMechanics")
	static void GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin);
	
	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayMechanics")
	static void GetClosestTargetsMax(int32 MaxTargets, const FVector& Origin, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets);

	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayMechanics")
	static void GetClosestTargets(const FVector& Origin, const TArray<AActor*>& Actors, TArray<AActor*>& OutSortedTargets);
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "RPGAbilitySystemLibrary|GameplayMechanics", meta = (DefaultToSelf = "Instigator"))
	static bool TraceAttackTrajectory(AActor* Instigator, 
									  const FGameplayTag& StartSocketTag, const FVector& TargetLocation, 
									  float TraceRadius , ETraceTypeQuery TraceChannel,FHitResult& OutHitResult);
	
	UFUNCTION(BlueprintCallable,Category = "RPGAbilitySystemLibrary|GameplayMechanics|Projectile", meta = (DefaultToSelf = "WorldContextObject"))
	static ARPGProjectile* SpawnProjectileTowardsTarget(const UObject* WorldContextObject, TSubclassOf<ARPGProjectile> ProjectileClass,
	                                                    const FVector& SpawnLocation, const FVector& TargetLocation,bool bOverridePitch, float PitchOverride, 
	                                                    AActor* Instigator, const FDamageEffectParams& DamageEffectParams);
	
	UFUNCTION(BlueprintCallable,Category = "RPGAbilitySystemLibrary|GameplayMechanics|Projectile", meta = (DefaultToSelf = "WorldContextObject"))
	static ARPGProjectile* SpawnProjectileInDirection(const UObject* WorldContextObject,TSubclassOf<ARPGProjectile> ProjectileClass,
													const FVector& SpawnLocation, const FRotator& SpawnRotation,bool bOverridePitch, float PitchOverride, 
													AActor* Instigator, const FDamageEffectParams& DamageEffectParams);

	/*
	 * Math Algorithm
	 */

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|Math Algorithm")
	static TArray<FRotator> EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators);
	
	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|Math Algorithm")
	static TArray<FVector> EvenlySpacedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors);
	
	/**
	 * 圆形径向地表阵列生成�?(法阵/碎片环形专用分布算法)
	 * @param CenterLocation  中心原点
	 * @param NumPoints		  生成的点�?
	 * @param Radius		  外环半径
	 * @param YawOverride	  可选的整体旋转偏移角度
	 * @param bIncludeCenter  是否在圆心强制留存一个落�?
	 */
	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|Math Algorithm", meta = (DefaultToSelf = "WorldContextObject"))
	static TArray<FVector> GetGroundRadialPoints(const UObject* WorldContextObject, const FVector& CenterLocation, int32 NumPoints, float Radius, float YawOverride = 0.f, bool bIncludeCenter = true);

	/*
	 * Damage Effect
	 */

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|DamageEffect")
	static FGameplayEffectSpecHandle MakeDamageEffectSpec(const FDamageEffectParams& DamageEffectParams);

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|DamageEffect")
	static float GetDamageFalloff(const FVector& Origin, const AActor* TargetActor, float InnerRadius, float OuterRadius);

	/*
	 * Effect Context Getters
	 */

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static bool IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle);
	
	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle);
	
	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static bool IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static TSubclassOf<UGameplayEffect> GetDebuffCarrier(const FGameplayEffectContextHandle& EffectContextHandle);
	
	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static float GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static float GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static float GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static bool IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static float GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static float GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static FVector GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static FGameplayTag GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static FGameplayTag GetDebuffType(const FGameplayEffectContextHandle& EffectContextHandle);

	/*
	 * Effect Context Setters
	 */

	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static void SetIsBlockedHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit);
	
	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static void SetIsCriticalHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit);
	
	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static void SetIsSuccessfulDebuff(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInSuccessfulDebuff);
	
	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static void SetDebuffCarrier(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle,TSubclassOf<UGameplayEffect> DebuffCarrierClass);
	
	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static void SetDebuffDamage(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InDamage);
	
	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static void SetDebuffDuration(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InDuration);
	
	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static void SetDebuffFrequency(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InFrequency);

	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static void SetIsRadialDamage(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsRadialDamage);

	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static void SetRadialDamageInnerRadius(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InInnerRadius);

	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static void SetRadialDamageOuterRadius(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InOuterRadius);

	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayEffects")
	static void SetRadialDamageOrigin(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FVector& InOrigin);

	static void SetDamageType(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FGameplayTag& InDamageType);
	
	static void SetDebuffType(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FGameplayTag& InDebuffType);

	/*
	 * Gameplay Tags
	 */

	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayTags")
	static void ApplyLooseTagToActor(AActor* TargetActor, FGameplayTag TagToApply);
	
	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayTags")
	static void RemoveLooseTagFromActor(AActor* TargetActor, FGameplayTag TagToRemove);

	/*
	 * Gameplay Helpers
	 */

	UFUNCTION(BlueprintCallable, Category = "RPGAbilitySystemLibrary|GameplayHelpers")
	static FTaggedMontage PickRandomTaggedMontage (const TArray<FTaggedMontage>& Montages);
	
	/*
	 * Combat
	 */
	
	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|Combat")
	static USkeletalMeshComponent* GetWeapon(const AActor* CombatActor);
	
	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|Combat")
	static FVector GetCombatSocketLocation(const AActor* CombatActor, const FGameplayTag& MontageTag);

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|Combat")
	static TArray<FTaggedMontage> GetAttackMontages(const AActor* CombatActor);

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|Combat")
	static UAnimMontage* GetHitReactMontage(const AActor* CombatActor);

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|Combat")
	static UNiagaraSystem* GetHitReactEffect(const AActor* CombatActor);

	UFUNCTION(BlueprintPure, Category = "RPGAbilitySystemLibrary|Combat")
	static AActor* GetCombatTarget(const AActor* CombatActor);



	static void ApplyEffectToSelf(UAbilitySystemComponent* ASC, AActor* Avatar,
							  TSubclassOf<UGameplayEffect> EffectClass,
							  float Level);

};



