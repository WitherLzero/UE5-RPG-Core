// Copyright rynnli

#include "RPGFramework/Pooling/DamageTextPoolManager.h"

#include "RPGFramework/Pooling/DamageTextActor.h"
#include "GameFramework/PlayerController.h"

void UDamageTextPoolManager::Initialize(APlayerController* Owner, TSubclassOf<ADamageTextActor> InClass)
{
	OwnerController = Owner;
	DamageTextActorClass = InClass;
}

void UDamageTextPoolManager::PreWarm(int32 Count)
{
	if (!OwnerController || !DamageTextActorClass) return;

	UWorld* World = OwnerController->GetWorld();
	if (!World) return;

	for (int32 i = 0; i < Count; ++i)
	{
		ADamageTextActor* Actor = World->SpawnActorDeferred<ADamageTextActor>(
			DamageTextActorClass, FTransform::Identity);
		if (Actor)
		{
			Actor->SetPoolManager(this);
			Actor->FinishSpawning(FTransform::Identity);
			Actor->Released();	// start hidden (widget only)
			AvailablePool.Add(Actor);
		}
	}
}

ADamageTextActor* UDamageTextPoolManager::Acquire(const FVector& WorldLocation)
{
	if (!OwnerController || !DamageTextActorClass) return nullptr;

	ADamageTextActor* Actor = nullptr;

	if (AvailablePool.Num() > 0)
	{
		// Pop from available pool
		Actor = AvailablePool[0];
		AvailablePool.RemoveAt(0);
	}
	else
	{
		// Pool miss — spawn new if below max size
		if (ActivePool.Num() >= MaxPoolSize)
		{
			return nullptr;
		}

		UWorld* World = OwnerController->GetWorld();
		if (!World) return nullptr;

		Actor = World->SpawnActorDeferred<ADamageTextActor>(
			DamageTextActorClass, FTransform::Identity);
		if (!Actor) return nullptr;

		Actor->SetPoolManager(this);
		Actor->FinishSpawning(FTransform::Identity);
		Actor->Released();	// initial hidden state
	}

	// Position and activate
	Actor->SetActorLocation(WorldLocation);
	Actor->Acquired();

	ActivePool.Add(Actor);
	return Actor;
}

void UDamageTextPoolManager::Release(ADamageTextActor* Actor)
{
	if (!Actor) return;

	Actor->Released();

	ActivePool.Remove(Actor);
	if (!AvailablePool.Contains(Actor))
	{
		AvailablePool.Add(Actor);
	}
}

void UDamageTextPoolManager::Shutdown()
{
	UWorld* World = OwnerController ? OwnerController->GetWorld() : nullptr;
	if (World)
	{
		for (auto& Actor : ActivePool)
		{
			if (Actor) World->DestroyActor(Actor);
		}
		for (auto& Actor : AvailablePool)
		{
			if (Actor) World->DestroyActor(Actor);
		}
	}

	ActivePool.Empty();
	AvailablePool.Empty();
}
