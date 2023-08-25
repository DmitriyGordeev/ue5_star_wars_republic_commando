// Fill out your copyright notice in the Description page of Project Settings.

#include "AITaskManager.h"
#include "AIBaseTask.h"


// #include "SAdvancedTransformInputBox.h"
// #include "BehaviorTree/BehaviorTreeTypes.h"
// #include "Containers/Deque.h"
#include "Kismet/KismetSystemLibrary.h"


// TODO: сделать общие коментарии после тестов


UAITaskManager::UAITaskManager()
{
	ActiveTask = nullptr;
	
}

void UAITaskManager::Start()
{
	TaskQueue.Reserve(TaskQueueSize);
	
	bStarted = true;
	Recalculate();
}

void UAITaskManager::Recalculate()
{
	// UE_LOG(LogTemp, Log, TEXT("TaskManager::Recalculate()"));
	if (Tasks.IsEmpty())
		return;
	
	if (!CheckRecalculateCooldownIsReady())
	{
		return;
	}
	LastRecalcUnixTime = GetCurrentMilliseconds();

	UE_LOG(LogTemp, Log, TEXT("TaskManager::Recalculate() ready"));
	
	UAIBaseTask* Winner = FindWinnerTask();
	
	if (!Winner)
	{
		UE_LOG(LogTemp, Log, TEXT("TaskManager::Recalculate() Winner is null"));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[Loop] Winner = %s"), *Winner->GetName());
	
	if (Winner->GetProba() <= 0.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("TaskManager::Recalculate() Winner(%s)->GetProba = 0.0f"), *Winner->GetName());
		return;
	}
	
	if (ActiveTask)
	{
		UE_LOG(LogTemp, Log, TEXT("TaskManager::ActiveTask %s"), *ActiveTask->GetName());

		// if task was running before this recalculation, and is the winner again -
		// should we ask it for interruption or not (depends on bShouldRestartIfWinnerAgain boolean check in BP)
		// if bShouldRestartIfWinnerAgain = false - task won't be interrupted and continue running
		if (Winner == ActiveTask)
		{
			UE_LOG(LogTemp, Log, TEXT("TaskManager: Winner == ActiveTask %s"), *ActiveTask->GetName());
			if (ActiveTask->IsRunning() && !ActiveTask->bShouldRestartIfWinnerAgain)
				return;
		}

		// Try to interrupt running task if was asked, and
		// Reset() its state if Completed / Interrupted = true
		if (ActiveTask->IsRunning())
		{
			UE_LOG(LogTemp, Log, TEXT("TaskManager::ActiveTask %s IsRunning() = true"), *ActiveTask->GetName());
			if (!TryInterruptActiveTask())
				return;
		}
		else if (ActiveTask->IsCompleted() || ActiveTask->IsInterrupted())
		{
			UE_LOG(LogTemp, Log, TEXT("TaskManager::ActiveTask( %s )->IsCompleted() or IsInterrupted() = true"), *ActiveTask->GetName());
			ActiveTask->Reset();
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("TaskManager::Recalculate() Winner Task Name = %s"), *Winner->GetName());

	// Cleanup all reaction from Reactions map which were marked Consumed or expired
	CleanupReactions();
	
	// Update TaskQueue
	if (ActiveTask)
	{
		// TODO: Enqueue() ?
		TaskQueue.AddFront(ActiveTask);
		if (TaskQueue.Num() > TaskQueueSize)
			TaskQueue.Pop();
	}
	
	ActiveTask = Winner;
	UE_LOG(LogTemp, Log, TEXT("Selecting task %s as Winner"), *ActiveTask->GetName());
	
	Winner->SelectAsWinner(GetCurrentMilliseconds());
	Winner->Start();
}

bool UAITaskManager::TryInterruptActiveTask()
{
	UE_LOG(LogTemp, Log, TEXT("TaskManager::RequestInterruptActive"));
	bWaitingForActiveTaskInterrupted = true;
	if (!ActiveTask)
		return true;

	if (AIOwner.IsValid())
	{
		ActiveTask->AskInterrupt(AIOwner.Get());
	}

	return ActiveTask->IsInterrupted() || ActiveTask->IsCompleted();
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


void UAITaskManager::ConsumeReaction(int32 ReactionType, int64 LifeTimeMs)
{	
	LifeTimeMs = LifeTimeMs < 0 ? 0 : LifeTimeMs;
	
	Reactions.Emplace(
		ReactionType,
		MakeShared<AIReaction>(
			ReactionType,
			false,
			GetCurrentMilliseconds(),
			LifeTimeMs
	));

	UE_LOG(LogTemp, Log, TEXT("Consuming reaction type = %i, StartTime = %lld"), ReactionType, GetCurrentMilliseconds());
	Recalculate();
}

void UAITaskManager::Tick(float DeltaTime)
{
	// TODO: expose tick frequency float (time sec)
	
	if (!bStarted)
		return;

	if (ActiveTask)
		UE_LOG(LogTemp, Log, TEXT("TaskManager tick | active task name = %s"), *ActiveTask->GetName());

	if (!ActiveTask)
	{
		Recalculate();
		return;
	}

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
	if (LastRecalcUnixTime == 0)
	{
		LastRecalcUnixTime = GetCurrentMilliseconds();
		return true;
	}
	return (GetCurrentMilliseconds() - LastRecalcUnixTime) > UpdateFreqMs;
}

bool UAITaskManager::TryActivateReaction(UAIBaseTask* FromTask, int32 ReactionType)
{
	if (Reactions.Contains(ReactionType))
	{
		if (FromTask)
			FromTask->SetConsumedReaction(true);
		Reactions[ReactionType]->Consumed = true;
		return true;
	}
	return false;
}

TTuple<UAIBaseTask*, int> UAITaskManager::CompareTwoTasks(UAIBaseTask* T1, UAIBaseTask* T2, int Index1, int Index2)
{
	if (!T1 || !T2)
		return {nullptr, -1};

	const float Proba1 = T1->GetProba();
	const float Proba2 = T2->GetProba();
	UE_LOG(LogTemp, Log, TEXT("CompareTwoTasks(%s, %s): proba1 = %f, proba2 = %f"),
		*T1->GetName(),
		*T2->GetName(),
		Proba1,
		Proba2);

	if (Proba1 > Proba2)
	{
		UE_LOG(LogTemp, Log, TEXT("T1(%s) is Winner"), *T1->GetName());
		T2->SetConsumedReaction(false);
		return {T1, Index1};
	}
	
	if (Proba1 == Proba2)
	{
		
		auto Pair = PriorityMatrix.Find(TTuple<int, int>(Index1, Index2));
		if (Pair && *Pair < 0)
		{
			if (*Pair < 0)
			{
				UE_LOG(LogTemp, Log, TEXT("Selecting with Priority matrix T2(%s) is Winner"), *T2->GetName());
				T1->SetConsumedReaction(false);
				return {T2, Index2};
			}
				
			if (*Pair > 0)
			{
				UE_LOG(LogTemp, Log, TEXT("Selecting with Priority matrix T1(%s) is Winner"), *T1->GetName());
				T2->SetConsumedReaction(false);
				return {T1, Index1};
			}
				
		}

		const auto TossedValue = FMath::FRand();
		UE_LOG(LogTemp, Log, TEXT("Selecting random task of two, p = %f"), TossedValue);

		const float ProbaSum = T1->GetProba() + T2->GetProba();
		if (ProbaSum == 0.0f)
		{
			UE_LOG(LogTemp, Log, TEXT("Probasum = 0.0f -> T1(%s) is Winner"), *T1->GetName());
			return {T1, Index1};
		}
			

		// TODO: протестировать
		const float P1 = T1->GetProba() / ProbaSum;
		if (TossedValue < P1)
		{
			UE_LOG(LogTemp, Log, TEXT("\tP1(normalized) = %f -> T1(%s) is Winner"), P1, *T1->GetName());
			T2->SetConsumedReaction(false);
			return {T1, Index1};
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("\tP2(normalized) = %f -> T2(%s) is Winner"), (1.0f - P1), *T2->GetName());
			T1->SetConsumedReaction(false);
			return {T2, Index2};
		}
	}

	T1->SetConsumedReaction(false);
	UE_LOG(LogTemp, Log, TEXT("\tDefault return T2(%s)"), *T2->GetName());
	return {T2, Index2};
}

void UAITaskManager::CleanupReactions()
{
	const int64 TimeNow = GetCurrentMilliseconds();
	for(const auto& t : Reactions)
	{
		if (t.Value->Consumed || TimeNow >= t.Value->LifeTimeMs + t.Value->StartTime)
		{
			UE_LOG(LogTemp, Log, TEXT("TaskManager::Recalculate() EReaction %i is to be removed"), t.Key);
			Reactions.Remove(t.Key);
		}
	}
}

int64 UAITaskManager::GetCurrentMilliseconds()
{
	const auto CurrentTime = FDateTime::Now();
	return CurrentTime.GetMillisecond() +
			CurrentTime.GetSecond() * 1000 +
			CurrentTime.GetMinute() * 60 * 1000;
}

UAIBaseTask* UAITaskManager::GetPreviousActionFromQueue(int32 PreviousActionIndex)
{
	if (PreviousActionIndex >= 0)
		return ActiveTask;

	if (PreviousActionIndex < -TaskQueue.Num())
		return nullptr;

	return TaskQueue[TaskQueue.Num() + PreviousActionIndex];
}

TArray<int32> UAITaskManager::GetUnconsumedReactions() const
{
	TArray<int32> output;
	Reactions.GetKeys(output);
	return output;
}

UAIBaseTask* UAITaskManager::FindWinnerTask()
{
	UAIBaseTask* Winner = nullptr;
	int WinnerIndex = -1;
	
	for(auto i = 0; i < Tasks.Num(); i++)
	{
		// early stop for comparing the task with others if specified explicitly (in BP)
		if (Tasks[i]->ShouldBeIgnored(AIOwner.Get()))
		{
			UE_LOG(LogTemp, Log, TEXT("TaskManager::Recalculate() Task %s ShouldBeIgnored = true"), *Tasks[i]->GetName());
			continue;
		}
		
		Tasks[i]->ExtractProba(AIOwner.Get());
		if (Tasks[i]->GetProba() == 0.0f)
			continue;
		
		UE_LOG(LogTemp, Log, TEXT("TaskManager::Recalculate() Task %s -> ExtractProba = %f"), *Tasks[i]->GetName(), Tasks[i]->GetProba());
		
		if (!Winner)
		{
			Winner = Tasks[i];
			WinnerIndex = i;
			continue;
		}

		UE_LOG(LogTemp, Log,
			TEXT("TaskManager::Recalculate() Task %s : GetConsumedReaction() = %i"),
			*Tasks[i]->GetName(),
			Tasks[i]->GetConsumedReaction()
			);

		if (Tasks[i]->GetConsumedReaction())
		{
			// if both winner and current task have consumed some reaction, we need
			// to treat them as equal
			if (Winner->GetConsumedReaction())
			{
				TTuple<UAIBaseTask*, int> Tuple = CompareTwoTasks(Tasks[i], Winner, i, WinnerIndex);
				Winner = Tuple.Key;
				WinnerIndex = Tuple.Value;
				continue;
			}

			// if Winner hasn't consumed any reaction,
			// current task Tasks[i] should have more priority
			// (except only if Tasks[i] itself has Proba = 0)
			if (Tasks[i]->GetProba() > 0.0f)
			{
				Winner = Tasks[i];
				WinnerIndex = i;
				continue;
			}
		}

		// If current task Tasks[i] hasn't consumed any reaction,
		// but winner has, Winner has more priority
		if (Winner->GetConsumedReaction())
			continue;
		
		// If both haven't consumed any reaction - compare them as equal
		TTuple<UAIBaseTask*, int> Tuple = CompareTwoTasks(Tasks[i], Winner, i, WinnerIndex);
		Winner = Tuple.Key;
		WinnerIndex = Tuple.Value;
	}

	return Winner;
}
