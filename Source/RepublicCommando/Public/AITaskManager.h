// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIBaseTask.h"
#include "AIController.h"
#include "AIReaction.h"
#include "Containers/RingBuffer.h"
#include "Templates/Tuple.h"
#include "AITaskManager.generated.h"


class AAIController;

/**
 * 
 */
UCLASS(Blueprintable)
class REPUBLICCOMMANDO_API UAITaskManager : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UAITaskManager();
	// UAITaskManager(const FObjectInitializer&);
	
	// нужно было включить AIModule в *.Build.cs
	// иначе доступ к AAIController вызовет 'unresolved external symbol error'
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AAIController> AIOwner;
	
	/* TaskManager's entry point (equivalent of RunBehaviorTree) */
	UFUNCTION(BlueprintCallable)
	virtual void Start();

	/* Find next relevant task to execute */
	UFUNCTION(BlueprintCallable)
	virtual void Recalculate();
	
	/* Asks current task for interruption - it's up to Task to respond:
	 * if task hasn't responded - it's continue running until marked Completed */
	UFUNCTION(BlueprintCallable)
	virtual bool TryInterruptActiveTask();
	
	// virtual void Tick();
	
	// TODO: OnBeforeTaskChange ?
	// TODO: OnAfterTaskChange ?

	/** Adds task to TaskManager
	 * returns index of just added task in the array of tasks,
	 * if task null object returns -1
	 * */
	UFUNCTION(BlueprintCallable)
	UPARAM(DisplayName = "TaskIndex") int AddTask(UAIBaseTask* Task);
	
	// TODO: move to protected
	UPROPERTY(BlueprintReadWrite)
	UAIBaseTask* ActiveTask;

	UFUNCTION(BlueprintCallable)
	void ConsumeReaction(int32 ReactionType, int64 LifeTimeMs = 0);

	// -------- FTickableGameObject functions --------------
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableInEditor() const override;
	virtual bool IsTickableWhenPaused() const override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetWorld() const override;

	/** Tells TaskManager that Task with TaskIndex1 should have greater priority
	 * over TaskIndex2 in case both tasks have the same probas */
	UFUNCTION(BlueprintCallable)
	virtual void AddPairWisePriority(int HigherPriorityTaskIndex, int LowerPriorityTaskIndex);
	
	UFUNCTION(BlueprintCallable)
	bool FindReaction(int32 EnumIndex) const { return Reactions.Contains(EnumIndex); }
	
	UFUNCTION(BlueprintCallable)
	bool TryActivateReaction(UAIBaseTask* FromTask, int32 EnumIndex);

	bool CheckRecalculateCooldownIsReady();
	
	static int64 GetCurrentMilliseconds();

	/* Extracts Task that was executed NumActionBefore and is
	 * currently stored in Queue.
	 * @NumActionBefore:
	 *		>= 0 is the current task,
	 *		-1 is the previous task finished or interrupted,
	 *		-2 is the task before the previous one, etc.
	 * @Return: pointer to task of nullptr if queue is empty or index out of bounds		
	 */
	UFUNCTION(BlueprintCallable)
	UAIBaseTask* GetPreviousActionFromQueue(int32 PreviousActionIndex);

	UFUNCTION(BlueprintCallable)
	TArray<int32> GetUnconsumedReactions() const;
	
protected:
	TTuple<UAIBaseTask*, int> CompareTwoTasks(UAIBaseTask* T1, UAIBaseTask* T2, int Index1, int Index2);

	/* Loops through available tasks and identify current winner -
	 * task with the highest probability or random,
	 * if more than one are equally probable */
	UAIBaseTask* FindWinnerTask();
	
	/* Removes reations which are marked as
	 * Consumed=True or expired by its lifetime */
	void CleanupReactions();
	
protected:
	UPROPERTY(BlueprintReadWrite)
	TArray<UAIBaseTask*> Tasks;
	
	bool bStarted {false};

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int32 UpdateFreqMs {0};

	UPROPERTY(BlueprintReadOnly)
	bool bWaitingForActiveTaskInterrupted {false};

	// pair-wise priority for each task
	TMap<TTuple<int, int>, int> PriorityMatrix;

	/* Reactions currently experienced by AI and are waiting
	 * to be consumed by Tasks
	 * @key = Index of reaction (can be created from Enum class by gamedesigner in BPs)
	 * @value = AIReaction instance
	 */
	TMap<int32, TSharedPtr<AIReaction>> Reactions;
	
	int64 LastRecalcUnixTime {0};

	// Stores previously executed tasks,
	// the end of the queue is the task executed right
	// before currently ActiveTask
	TRingBuffer<UAIBaseTask*> TaskQueue;
		
	// How much tasks to save up in the queue.
	// By default = 0 meaning we don't want to store any
	// information about previously executed tasks
	// todo: добавить UPROP на макс / мин значения ?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int TaskQueueSize {0};
};
