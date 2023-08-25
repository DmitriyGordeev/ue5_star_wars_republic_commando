// Fill out your copyright notice in the Description page of Project Settings.

#include "AIBaseTask.h"
#include "AITaskManager.h"


// TODO: сделать общие коментарии после тестов


void UAIBaseTask::Start()
{
	Reset();
	UE_LOG(LogTemp, Log, TEXT("UAIBaseTask::Start()"));
	bRunning = true;
	OnExecute(GetAIController());
}

float UAIBaseTask::FindProba_Implementation(AAIController* Controller)
{
	Proba = 1.0f;
	return Proba;
}

bool UAIBaseTask::ShouldBeIgnored_Implementation(AAIController* Controller)
{
	return false;
}

void UAIBaseTask::Reset()
{
	bRunning = false;
	bCompleted = false;
	bInterrupted = false;
	bAskedForInterruption = false;
	UE_LOG(LogTemp, Log, TEXT("Reset() task %s"), *GetName());
}

void UAIBaseTask::SetConsumedReaction(bool Consumed)
{
	ConsumedReaction = Consumed;
}

float UAIBaseTask::ExtractProba(AAIController* Controller)
{
	Proba = FindProba(Controller);
	Proba = Proba > 1.0f ? 1.0f : Proba;
	Proba = Proba < 0.0f ? 0.0f : Proba;
	return Proba;
}

void UAIBaseTask::AskInterrupt(AAIController* Controller)
{
	UE_LOG(LogTemp, Log, TEXT("Task %s -> AskInterrupt()"), *GetName());
	bAskedForInterruption = true;
	OnInterruptedResponse(Controller);
}

void UAIBaseTask::Tick(float DeltaTime)
{
	UE_LOG(LogTemp, Log, TEXT("UAIBaseTask Tick"));
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
	bCompleted = true;
	bRunning = false;
	UE_LOG(LogTemp, Log, TEXT("Task marked as completed"));
}


void UAIBaseTask::MarkInterrupted()
{
	bInterrupted = true;
	bRunning = false;	// todo: убрать флаг bRunning
	bAskedForInterruption = false;
	UE_LOG(LogTemp, Log, TEXT("Task marked as interrupted"));
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
	
	return nullptr;
}

bool UAIBaseTask::IsRunning() const
{
	UE_LOG(LogTemp, Log, TEXT("%s IsRunning() = %i"), *GetName(), bRunning);
	return bRunning;
}


bool UAIBaseTask::IsCompleted() const
{
	UE_LOG(LogTemp, Log, TEXT("IsCompleted() = %i"), bCompleted);
	return bCompleted;
}

bool UAIBaseTask::IsInterrupted() const
{
	UE_LOG(LogTemp, Log, TEXT("IsInterrupted() = %i"), bInterrupted);
	return bInterrupted;
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


bool UAIBaseTask::IsReadyToBeWinner(int32 NewTimeMs) const
{
	return NewTimeMs - LastWinningTimeMs > WinnerCooldownTimeMs;
}

void UAIBaseTask::SelectAsWinner(int32 NewTimeMs)
{
	LastWinningTimeMs = NewTimeMs;
}
