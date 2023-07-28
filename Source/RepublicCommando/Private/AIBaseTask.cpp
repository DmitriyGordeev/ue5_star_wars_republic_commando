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

float UAIBaseTask::FindProba_Implementation(AAIController* Controller, UObject* ContextData)
{
	Proba = 1.0f;
	return Proba;
}

// float UAIBaseTask::FindProba_Implementation(AAIController* Controller, UObject* ContextData)
// {
// 	Proba = 1.0f;
// 	return Proba;
// }

void UAIBaseTask::Reset()
{
	bRunning = false;
	bCompleted = false;
	bInterrupted = false;
	UE_LOG(LogTemp, Log, TEXT("Reset() task %s"), *GetName());
}

float UAIBaseTask::ExtractProba(AAIController* Controller, UObject* ContextData)
{
	Proba = FindProba(Controller, ContextData);
	Proba = Proba > 1.0f ? 1.0f : Proba;
	Proba = Proba < 0.0f ? 0.0f : Proba;
	UE_LOG(LogTemp, Log, TEXT("Clamped Proba = %f"), Proba);
	return Proba;
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
	UE_LOG(LogTemp, Log, TEXT("Task marked as completed"));
	
	// TODO: notify up to TaskManager ?
}


void UAIBaseTask::MarkInterrupted()
{
	bInterrupted = true;
	UE_LOG(LogTemp, Log, TEXT("Task marked as interrupted"));
	
	// TODO: notify up to TaskManager ?
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
			UE_LOG(LogTemp, Display, TEXT("AIBaseTask::GetWorld() => Sucessfully GetWorld()"));
			return Outer->GetWorld();
		}
	}
	
	// Else return null - the latent action will fail to initialize
	UE_LOG(LogTemp, Display, TEXT("World is null"));
	return nullptr;
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

float UAIBaseTask::GetProba() const
{
	UE_LOG(LogTemp, Log, TEXT("UAIBaseTask::GetProba() = %f"), Proba);
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

