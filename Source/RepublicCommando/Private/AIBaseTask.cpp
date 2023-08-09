// Fill out your copyright notice in the Description page of Project Settings.

#include "AIBaseTask.h"
#include "AITaskManager.h"


// TODO: сделать общие коментарии после тестов


void UAIBaseTask::Start()
{
	Reset();
	bRunning = true;
	OnExecute(GetAIController());
}

float UAIBaseTask::FindProba_Implementation(AAIController* Controller, UObject* ContextData)
{
	Proba = 1.0f;
	return Proba;
}

void UAIBaseTask::Reset()
{
	bRunning = false;
	bCompleted = false;
	bInterrupted = false;
	bAskedForInterruption = false;
	// UE_LOG(LogTemp, Log, TEXT("Reset() task %s"), *GetName());
}

bool UAIBaseTask::IsReadyToBeWinner(int32 NewTimeMs) const
{
	return NewTimeMs - LastWinningTimeMs > WinnerCooldownTimeMs;
}

void UAIBaseTask::SelectAsWinner(int32 NewTimeMs)
{
	// UE_LOG(LogTemp, Log, TEXT("Task %s selected as winner at %i"), *GetName(), NewTimeMs);
	LastWinningTimeMs = NewTimeMs;
}

float UAIBaseTask::ExtractProba(AAIController* Controller, UObject* ContextData)
{
	Proba = FindProba(Controller, ContextData);
	Proba = Proba > 1.0f ? 1.0f : Proba;
	Proba = Proba < 0.0f ? 0.0f : Proba;
	return Proba;
}

void UAIBaseTask::AskInterrupt(AAIController* Controller)
{
	bAskedForInterruption = true;
	OnInterruptedResponse(Controller);
}

void UAIBaseTask::Tick(float DeltaTime)
{
	// TODO: if (!Interrupted() and !Completed()) ?
	OnTick(GetAIController());
}

bool UAIBaseTask::IsTickable() const
{
	bool TickableCheck = !IsTemplate(RF_ClassDefaultObject) && bRunning && !bCompleted && !bInterrupted;
	// UE_LOG(LogTemp, Log, TEXT("%s !CDO = %i"), *GetName(), !IsTemplate(RF_ClassDefaultObject));
	// UE_LOG(LogTemp, Log, TEXT("%s bRunning = %i"), *GetName(), bRunning);
	// UE_LOG(LogTemp, Log, TEXT("%s !bCompleted = %i"), *GetName(), !bCompleted);
	// UE_LOG(LogTemp, Log, TEXT("%s !bInterrupted = %i"), *GetName(), !bInterrupted);
	
	return TickableCheck;
}

bool UAIBaseTask::IsTickableInEditor() const
{
	return false;
}

bool UAIBaseTask::IsTickableWhenPaused() const
{
	return false;
}

void UAIBaseTask::MarkCompleted()
{
	bRunning = false;
	bCompleted = true;
	// UE_LOG(LogTemp, Log, TEXT("%s Task marked as completed"), *GetName());
}

void UAIBaseTask::MarkInterrupted()
{
	bRunning = false;
	bInterrupted = true;
	// UE_LOG(LogTemp, Log, TEXT("%s Task marked as interrupted"), *GetName());
	bAskedForInterruption = false;
}

TStatId UAIBaseTask::GetStatId() const
{
	return TStatId();
}

UWorld* UAIBaseTask::GetWorld() const
{
	// return GetOuter()->GetWorld();
	
	// Return null if called from the CDO, or if the outer is being destroyed
	if (!HasAnyFlags(RF_ClassDefaultObject) &&
		!GetOuter()->HasAnyFlags(RF_BeginDestroyed) &&
		!GetOuter()->IsUnreachable())
	{
		// Try to get the world from the owning actor if we have one
		AActor* Outer = GetTypedOuter<AActor>();
		if (Outer != nullptr)
		{
			return Outer->GetWorld();
		}
	}
	
	// Else return null - the latent action will fail to initialize
	// UE_LOG(LogTemp, Display, TEXT("World is null"));
	return nullptr;
}

bool UAIBaseTask::IsRunning() const
{
	// UE_LOG(LogTemp, Log, TEXT("%s IsRunning() = %i"), *GetName(), bRunning);
	return bRunning;
}

bool UAIBaseTask::IsCompleted() const
{
	// UE_LOG(LogTemp, Log, TEXT("%s IsCompleted() = %i"), *GetName(), bCompleted);
	return bCompleted;
}

bool UAIBaseTask::IsInterrupted() const
{
	// UE_LOG(LogTemp, Log, TEXT("%s IsInterrupted() = %i"), *GetName(), bInterrupted);
	return bInterrupted;
}

float UAIBaseTask::GetProba() const
{
	// UE_LOG(LogTemp, Log, TEXT("%s GetProba() = %f"), *GetName(), Proba);
	return Proba;
}

void UAIBaseTask::SetTaskManager(UAITaskManager* OwnerTaskManager)
{
	TaskManager = OwnerTaskManager;
}

AAIController* UAIBaseTask::GetAIController()
{
	if (!TaskManager.IsValid())
		return nullptr;

	return TaskManager->AIOwner.Get();
}

