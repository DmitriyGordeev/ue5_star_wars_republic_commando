// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseNPCCharacter.h"
#include "BaseCloneCharacter.generated.h"


/**
 * 
 */
UCLASS()
class REPUBLICCOMMANDO_API ABaseCloneCharacter : public ABaseNPCCharacter
{
	GENERATED_BODY()

public:
	ABaseCloneCharacter();

	
	virtual FVariant GetSelectedEQSLocation();

	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
	bool InCover;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
	AActor* TargetEnemy;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
	TArray<ABaseCloneCharacter*> TeamMembers;

	
};
