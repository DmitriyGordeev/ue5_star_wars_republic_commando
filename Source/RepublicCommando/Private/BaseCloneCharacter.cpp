// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCloneCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"


// Sets default values
ABaseCloneCharacter::ABaseCloneCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

/**
 * @brief Returns location of selected EQS point, or false if wasn't selected
 * @return FVariant : FVector or bool in case BB value wasn't set or other error
 */
FVariant ABaseCloneCharacter::GetSelectedEQSLocationFromBB()
{
	const UBlackboardComponent* Blackboard = UAIBlueprintHelperLibrary::GetBlackboard(this);
	const FName BBVectorName(TEXT("EQS_Point"));
	
	// if (Blackboard)
	// {
	// 	FString BlackboardName = Blackboard->GetName();
	// 	UE_LOG(LogTemp, Display, TEXT("[ABaseCloneCharacter] BlackboardName = %s"), *BlackboardName);
	// 	bool IsEQSPointSet = Blackboard->IsVectorValueSet(BBVectorName);
	// 	
	// 	UE_LOG(LogTemp, Display, TEXT("[ABaseCloneCharacter] is EQS Point set = %i"), IsEQSPointSet);
	// 	if (IsEQSPointSet)
	// 	{
	// 		FVector Loc = Blackboard->GetValueAsVector(BBVectorName);
	// 		UE_LOG(LogTemp, Display, TEXT("[ABaseCloneCharacter] EQS_Point = %s"), *Vector.ToString());
	// 		return Loc;
	// 	}
	// 	
	// 	return false;
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Display, TEXT("Blackboard is null"));
	// }
	//
	// return false;

	if (!Blackboard)
	{
		UE_LOG(LogTemp, Log, TEXT("[ABaseCloneCharacter] Blackboard is not set"));
		return false;
	}
	
	bool IsEQSPointSet = Blackboard->IsVectorValueSet(BBVectorName);
	if (!IsEQSPointSet)
	{
		UE_LOG(LogTemp, Log, TEXT("[ABaseCloneCharacter] Blackboard vector EQS_Point was not set"));
		return false;
	}

	FVector Loc = Blackboard->GetValueAsVector(BBVectorName);
	UE_LOG(LogTemp, Log, TEXT("[ABaseCloneCharacter] EQS_Point = %s"), *Loc.ToString());
	return Loc;
}

FVector ABaseCloneCharacter::GetSelectedEQSLocation() const
{
	return EQSLocation;
}
