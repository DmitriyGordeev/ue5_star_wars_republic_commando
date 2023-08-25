// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// #include "AITaskManager.h"
#include "UObject/NoExportTypes.h"
#include "AIBaseTask.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEventDelegate);

class AAIController;
class UAITaskManager;

/**
 * 
 */
UCLASS(Blueprintable)
class REPUBLICCOMMANDO_API UAIBaseTask : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnExecute(AAIController* Controller);

	UFUNCTION(BlueprintImplementableEvent)
	void OnTick(AAIController* Controller);

	UFUNCTION(BlueprintImplementableEvent)
	void OnInterruptedResponse(AAIController* Controller);
	
	UFUNCTION(BlueprintCallable)
	virtual void Start();
	
	UFUNCTION(BlueprintNativeEvent)
	float FindProba(AAIController* Controller);

	UFUNCTION(BlueprintNativeEvent)
	bool ShouldBeIgnored(AAIController* Controller);
	
	/** Simple wrapper-setter around blueprint-implementable FindProba()
	 * this function forces to set Proba field after calculation, which BP user can forget to do */
	UFUNCTION()
	float ExtractProba(AAIController* Controller);
	
	UFUNCTION()
	void AskInterrupt(AAIController* Controller);
	
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableInEditor() const override;
	virtual bool IsTickableWhenPaused() const override;

	UFUNCTION(BlueprintCallable)
	virtual void MarkCompleted();

	UFUNCTION(BlueprintCallable)
	virtual void MarkInterrupted();

	virtual TStatId GetStatId() const override;
	virtual UWorld* GetWorld() const override;

	virtual bool IsRunning() const;
	virtual bool IsCompleted() const;
	virtual bool IsInterrupted() const;

	virtual float GetProba() const { return Proba; }

	UFUNCTION()
	void SetTaskManager(UAITaskManager* TaskManager);
	
	UFUNCTION(BlueprintCallable)
	AAIController* GetAIController();

	virtual void Reset();

	/** Should we stop and restart this task if
	 * we did recalculation and this task is still the most relevant among all?
	 * If 'false' - task will continue running without stops until MarkComplete or MarkInterrupted
	 * */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bShouldRestartIfWinnerAgain {true};

	UFUNCTION()
	void SetConsumedReaction(bool Consumed);

	UFUNCTION(BlueprintCallable)
	bool GetConsumedReaction() const { return ConsumedReaction; }

	/** Imposes cooldown on the task -
	 * prevents from picking the same Task too often if necessary */
	bool IsReadyToBeWinner(int32 NewTimeMs) const;
	
	void SelectAsWinner(int32 NewTimeMs);
	
protected:
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UAITaskManager> TaskManager;

	UPROPERTY(BlueprintReadOnly)
	bool bRunning {false};

	UPROPERTY(BlueprintReadOnly)
	bool bCompleted {false};

	UPROPERTY(BlueprintReadOnly)
	bool bInterrupted {false};
	float Proba {0.0f};

	UPROPERTY(BlueprintReadOnly)
	bool bAskedForInterruption {false};

	UPROPERTY(BlueprintReadOnly)
	bool ConsumedReaction {false};

	/** How often can be picked as a winner, specified in milliseconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0, UIMin=0))
	int32 WinnerCooldownTimeMs {0};
	
	int32 LastWinningTimeMs;
};
