// Fill out your copyright notice in the Description page of Project Settings.

#include "AITaskManager.h"
#include "AIBaseTask.h"


#include "SAdvancedTransformInputBox.h"
#include "Engine/RendererSettings.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Kismet/KismetSystemLibrary.h"


// TODO: сделать общие коментарии после тестов


void UAITaskManager::Start()
{
	bStarted = true;
	Recalculate();
}

// TODO: нужно отрефакторить
void UAITaskManager::Recalculate()
{
	UE_LOG(LogTemp, Log, TEXT("TaskManager::Recalculate()"));
	if (Tasks.IsEmpty())
		return;
	
	if (!CheckRecalculateCooldownIsReady())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("LastRecalcUnitTime = %lld"), LastRecalcUnixTime);

	LastRecalcUnixTime = FDateTime::Now().ToUnixTimestamp();
	
	UAIBaseTask* Winner = nullptr;
	float MaxProbaSoFar = -1.0f;
	float Proba = 0.0f;
	int WinnerIndex = -1;
	
	// TODO: распараллелить ? (могут быть проблемы, если FindProba пишет в ContextData)
	for(auto i = 0; i < Tasks.Num(); i++)
	{
		Proba = Tasks[i]->ExtractProba(AIOwner.Get(), ContextData);

		if (Proba > MaxProbaSoFar)
		{
			MaxProbaSoFar = Proba;
			Winner = Tasks[i];
			WinnerIndex = i;
		}
		else if (Proba == MaxProbaSoFar)
		{
			auto Pair = PriorityMatrix.Find(TTuple<int, int>(i, WinnerIndex));
			if (Pair && *Pair < 0)
			{
				UE_LOG(LogTemp, Log, TEXT("SORTING with PriorityMatrix = %i"), *Pair);
				continue;
			}

			// Randomly choose between two tasks if they have equal probabilities
			if (FMath::FRand() > 0.5f)
			{
				UE_LOG(LogTemp, Log, TEXT("Randomly choosing new task"));
				MaxProbaSoFar = Proba;
				Winner = Tasks[i];
				WinnerIndex = i;
			}
		}
	}
	
	if (!Winner)
	{
		UE_LOG(LogTemp, Log, TEXT("Winner is null"));
		return;
	}
	
	if (Winner->GetProba() <= 0.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("Winner->GetProba = 0.0f"));
		return;
	}
	
	if (ActiveTask && Winner == ActiveTask)
	{
		if (ActiveTask->bShouldRestartIfWinnerAgain)
		{
			UE_LOG(LogTemp, Log, TEXT("Restaring the same Task because it's a winner again"));
			ActiveTask->Reset();
			ActiveTask->Start();
		}
		return;
	}
	
	if (ActiveTask)
		ActiveTask->Reset();
	
	UE_LOG(LogTemp, Log, TEXT("Winner Task Name = %s"), *Winner->GetName());
	
	ActiveTask = Winner;
	Winner->Start();
}

bool UAITaskManager::TryInterruptActiveTask()
{
	UE_LOG(LogTemp, Log, TEXT("RequestInterruptActive"));
	bWaitingForActiveTaskInterrupted = true;
	if (!ActiveTask)
		return true;

	if (AIOwner.IsValid())
		ActiveTask->OnInterruptedResponse(AIOwner.Get());

	return ActiveTask->IsInterrupted();
}

int UAITaskManager::AddTask(UAIBaseTask* Task)
{
	if (!Task)	// TODO: добавить Task->IsValidLowLevel() ?
		return -1;
	Task->SetTaskManager(this);
	Tasks.Add(Task);
	UE_LOG(LogTemp, Log, TEXT("Task was added"));
	return Tasks.Num() - 1;
}


void UAITaskManager::Tick(float DeltaTime)
{
	// TODO: expose tick frequency float (time sec)
	
	if (!bStarted)
		return;

	if (ActiveTask)
		UE_LOG(LogTemp, Log, TEXT("TaskManager tick | active task name = %s"), *ActiveTask->GetName());

	if (!ActiveTask)
		return;

	if (ActiveTask->IsCompleted())
	{
		Recalculate();
		return;
	}

	if (bWaitingForActiveTaskInterrupted)
	{
		if (ActiveTask->IsCompleted() || ActiveTask->IsInterrupted())
		{
			UE_LOG(LogTemp, Log, TEXT("Task Interrupted"));
			bWaitingForActiveTaskInterrupted = false;
			Recalculate();
		}
	}
	
}

bool UAITaskManager::IsTickable() const
{
	return !IsTemplate(RF_ClassDefaultObject) && bStarted;
}

bool UAITaskManager::IsTickableInEditor() const
{
	return false;
}

bool UAITaskManager::IsTickableWhenPaused() const
{
	return false;
}

TStatId UAITaskManager::GetStatId() const
{
	return TStatId();
}

UWorld* UAITaskManager::GetWorld() const
{
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
	return nullptr;
}

void UAITaskManager::AddPairWisePriority(int HigherPriorityTaskIndex, int LowerPriorityTaskIndex)
{
	if (HigherPriorityTaskIndex >= Tasks.Num() || LowerPriorityTaskIndex >= Tasks.Num())
		return;

	if (HigherPriorityTaskIndex < 0 || LowerPriorityTaskIndex < 0)
		return;

	if (HigherPriorityTaskIndex == LowerPriorityTaskIndex)
		return;

	PriorityMatrix.Add(TTuple<int, int>(HigherPriorityTaskIndex, LowerPriorityTaskIndex), 1);
	PriorityMatrix.Add(TTuple<int, int>(LowerPriorityTaskIndex, HigherPriorityTaskIndex), -1);
}

bool UAITaskManager::CheckRecalculateCooldownIsReady()
{
	// TODO: перевести в милисекунды ? и добавить параметр частоты
	if (LastRecalcUnixTime == 0)
	{
		LastRecalcUnixTime = FDateTime::Now().ToUnixTimestamp();
		return true;
	}
	return (FDateTime::Now().ToUnixTimestamp() - LastRecalcUnixTime) > 1;
}
