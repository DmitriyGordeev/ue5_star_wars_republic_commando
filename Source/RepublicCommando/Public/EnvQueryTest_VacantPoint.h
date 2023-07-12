// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvQueryTest_VacantPoint.generated.h"

/**
 * 
 */
UCLASS()
class REPUBLICCOMMANDO_API UEnvQueryTest_VacantPoint : public UEnvQueryTest
{
	GENERATED_BODY()
	UEnvQueryTest_VacantPoint(const FObjectInitializer& ObjectInitializer);
	
	/** context */
	UPROPERTY(EditDefaultsOnly, Category=EmptyCategory)
	TSubclassOf<UEnvQueryContext_Querier> Querier;
	
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;

	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
};
