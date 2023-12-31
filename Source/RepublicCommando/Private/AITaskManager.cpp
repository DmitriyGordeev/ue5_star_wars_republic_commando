﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "AITaskManager.h"
#include "AIBaseTask.h"


// #include "SAdvancedTransformInputBox.h"
// #include "Engine/RendererSettings.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Kismet/KismetSystemLibrary.h"


// TODO: сделать общие коментарии после тестов


void UAITaskManager::Start()
{
	bStarted = true;
	Recalculate();
}

// TODO: нужно отрефакторить
void UAITaskManager::Recalculate(bool ShouldIgnoreCooldown)
{
	if (Tasks.IsEmpty())
		return;
	
	if (!ShouldIgnoreCooldown && !CheckRecalculateCooldownIsReady())
	{
		// UE_LOG(LogTemp, Log, TEXT("TaskManager::Recalculate() - cooldown block"));
		return;
	}

	LastRecalcUnixTimeMs = GetCurrentMilliseconds();
	// UE_LOG(LogTemp, Log, TEXT("LastRecalcUnixTimeMs = %i"), LastRecalcUnixTimeMs);
	
	UAIBaseTask* Winner = nullptr;
	float MaxProbaSoFar = -1.0f;
	float Proba = 0.0f;
	int WinnerIndex = -1;
	
	// TODO: распараллелить ? (могут быть проблемы, если FindProba пишет в ContextData) - потестить!
	for(auto i = 0; i < Tasks.Num(); i++)
	{
		if (!Tasks[i]->IsReadyToBeWinner(GetCurrentMilliseconds()))
			continue;
		
		Proba = Tasks[i]->ExtractProba(AIOwner.Get(), ContextData);
		if (Proba > MaxProbaSoFar)
		{
			MaxProbaSoFar = Proba;
			Winner = Tasks[i];
			WinnerIndex = i;
		}
		else if (Proba == MaxProbaSoFar)
		{
			// if probabilities of two tasks are equal we need to lookup to pair-wise priority
			// in case BP user forced one task to be always prevail after another
			auto IndexPair = PriorityMatrix.Find(TTuple<int, int>(i, WinnerIndex));
			if (IndexPair)
			{
				if (*IndexPair < 0)
					continue;
				
				MaxProbaSoFar = Proba;
				Winner = Tasks[i];
				WinnerIndex = i;
			}
			else
			{
				// Randomly choose between two tasks (winner-so-far and current one)
				// if they have equal probabilities
				if (FMath::FRand() > 0.5f)
				{
					MaxProbaSoFar = Proba;
					Winner = Tasks[i];
					WinnerIndex = i;
				}
			}
			
		}
	}
	
	if (!Winner)
		return;
	
	if (Winner->GetProba() <= 0.0f)
		return;
	
	if (ActiveTask)
	{
		if (Winner == ActiveTask)
		{
			if (ActiveTask->IsRunning() && !ActiveTask->bShouldRestartIfWinnerAgain)
				return;
		}

		if (ActiveTask->IsRunning())
		{
			if (!TryInterruptActiveTask())
				return;
		}
		else if (ActiveTask->IsCompleted() || ActiveTask->IsInterrupted())
		{
			ActiveTask->Reset();
		}
	}

	// Select new task and Start it
	ActiveTask = Winner;
	Winner->SelectAsWinner(GetCurrentMilliseconds());
	Winner->Start();
}

bool UAITaskManager::TryInterruptActiveTask()
{
	UE_LOG(LogTemp, Log, TEXT("RequestInterruptActive"));
	bWaitingForActiveTaskInterrupted = true;
	if (!ActiveTask)
		return true;

	if (AIOwner.IsValid())
	{
		ActiveTask->AskInterrupt(AIOwner.Get());
	}

	return ActiveTask->IsInterrupted();
}

int UAITaskManager::AddTask(UAIBaseTask* Task)
{
	if (!Task)
		return -1;
	Task->SetTaskManager(this);
	Tasks.Add(Task);
	UE_LOG(LogTemp, Log, TEXT("Task was added"));
	return Tasks.Num() - 1;
}


void UAITaskManager::Tick(float DeltaTime)
{
	if (!bStarted)
		return;

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
			
			// avoid cooldown check to catch just interrupted task as soon as possible
			Recalculate(true);
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
	if (LastRecalcUnixTimeMs == 0)
	{
		LastRecalcUnixTimeMs = GetCurrentMilliseconds();
		return true;
	}
	return (GetCurrentMilliseconds() - LastRecalcUnixTimeMs) > TaskRecalculationFrequencyMs;
}

int32 UAITaskManager::GetCurrentMilliseconds()
{
	const auto CurrentTime = FDateTime::Now();
	return CurrentTime.GetMillisecond() +
			CurrentTime.GetSecond() * 1000 +
			CurrentTime.GetMinute() * 60 * 1000;
}
