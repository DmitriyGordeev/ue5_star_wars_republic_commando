// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIBaseTask.h"
#include "HAL/ThreadManager.h"
#include "AIController.h"
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
	
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AAIController> AIOwner;
	
	/* TaskManager's entry point (equivalent of RunBehaviorTree) */
	UFUNCTION(BlueprintCallable)
	virtual void Start();

	/* Find next relevant task to execute */
	UFUNCTION(BlueprintCallable)
	virtual void Recalculate(bool ShouldIgnoreCooldown = false);
	
	/* Asks current task for interruption - it's up to Task to respond:
	 * if task hasn't responded - it's continue running until marked Completed */
	UFUNCTION(BlueprintCallable)
	virtual bool TryInterruptActiveTask();

	/** Adds task to TaskManager
	 * returns index of just added task in the array of tasks,
	 * if task null object returns -1
	 * */
	UFUNCTION(BlueprintCallable)
	UPARAM(DisplayName = "TaskIndex") int AddTask(UAIBaseTask* Task);
	
	// -------- FTickableGameObject functions --------------
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableInEditor() const override;
	virtual bool IsTickableWhenPaused() const override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetWorld() const override;
	
	UPROPERTY(BlueprintReadWrite)
	UObject* ContextData;

	/** Tells TaskManager that Task with TaskIndex1 should have greater priority
	 * over TaskIndex2 in case both tasks have the same probas */
	UFUNCTION(BlueprintCallable)
	virtual void AddPairWisePriority(int HigherPriorityTaskIndex, int LowerPriorityTaskIndex);
	
	bool CheckRecalculateCooldownIsReady();

	/** Extracts milliseconds part of current Date every hour
	 * ms + 1000 * s + 60 * 1000 * min */
	static int32 GetCurrentMilliseconds();
	
	/** Defines how often Recalculate() function can fire */
	UPROPERTY(BlueprintReadWrite, meta=(ClampMin=0, UIMin=0))
	int32 TaskRecalculationFrequencyMs {1000};
	
protected:
	/* Currently active task seleced for execution */
	UPROPERTY(BlueprintReadWrite)
	UAIBaseTask* ActiveTask;

	/* array of tasks to execute */
	UPROPERTY(BlueprintReadWrite)
	TArray<UAIBaseTask*> Tasks;
	
	bool bStarted {false};
	
	UPROPERTY(BlueprintReadOnly)
	bool bWaitingForActiveTaskInterrupted {false};

	/* pair-wise priority for each pair of tasks
	 * if tasks A and B have equal probabilities,
	 * then we can specify which one should be executed (has more priority)
	 * key: Tuple of (index of task A, index of task B)
	 * value: => 0 meaning A has more priority than B and vice-versa if < 0
	 */
	TMap<TTuple<int, int>, int> PriorityMatrix;

	/* Last time when Recalculate() function fired */
	int32 LastRecalcUnixTimeMs {0};
};
