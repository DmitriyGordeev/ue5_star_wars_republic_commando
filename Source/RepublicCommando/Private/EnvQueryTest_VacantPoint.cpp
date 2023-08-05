// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvQueryTest_VacantPoint.h"

#include "BaseCloneCharacter.h"
#include "ParticleHelper.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"


UEnvQueryTest_VacantPoint::UEnvQueryTest_VacantPoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Setup context object
	Querier = UEnvQueryContext::StaticClass();
	Cost = EEnvTestCost::Low;

	// Specify item type (Actor, Vector, todo: изменить?)
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
}


void UEnvQueryTest_VacantPoint::RunTest(FEnvQueryInstance& QueryInstance) const
{
    if (!Querier)
    {
	    UE_LOG(LogTemp, Display, TEXT("[VacantPoint::RunTest()] Run Test, Querier is not valid"));
    	return;
    }
	
	UE_LOG(LogTemp, Display, TEXT("[VacantPoint::RunTest()] Run Test, Context name = %s"), *(Querier->GetName()));

	// Get Owner of this EQS query
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr)
	{
		UE_LOG(LogTemp, Display, TEXT("[VacantPoint::RunTest()] QueryOwner is nullptr"));
		return;
	}

	// If it's not ABaseCloneCharacter we return
	ABaseCloneCharacter* CloneCharacter = Cast<ABaseCloneCharacter>(QueryOwner);
	if (CloneCharacter)
	{
		UE_LOG(LogTemp, Display, TEXT("[VacantPoint::RunTest()] Character.GetName() = %s"), *CloneCharacter->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("[VacantPoint::RunTest()] CloneCharacter is not valid"));
		return;
	}

	// Bind min-max scoring data (this is unused)
	// TODO: надо убрать, потому что скоринг не используется
	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	float MinThresholdValue = FloatValueMin.GetValue();

	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);
	float MaxThresholdValue = FloatValueMax.GetValue();

	
	TArray<FVector> ContextLocations;
	if (!QueryInstance.PrepareContext(Querier, ContextLocations))
	{
		UE_LOG(LogTemp, Warning, TEXT("[VacantPoint::RunTest()] QueryInstance.PrepareContext() returned false"));
		return;
	}

	for (FEnvQueryInstance::ItemIterator ItrEQSPoint(this, QueryInstance); ItrEQSPoint; ++ItrEQSPoint)
	{
		const FVector EQSItemLocation = GetItemLocation(QueryInstance, ItrEQSPoint.GetIndex());
		if (CloneCharacter->TeamMembers.IsEmpty())
		{
			UE_LOG(LogTemp, Display, TEXT("[VacantPoint::RunTest()] Team is Empty"));
			ItrEQSPoint.ForceItemState(EEnvItemStatus::Passed);
		}
		else
		{
			bool CurrentPointPassed = true;
			
			for(auto it = CloneCharacter->TeamMembers.begin();
				it != CloneCharacter->TeamMembers.end();
				++it)
			{
				const ABaseCloneCharacter* TeamMember = *it;
				if (TeamMember == nullptr)
					continue;
				
				FVector EQSLocation = TeamMember->GetSelectedEQSLocation();
				UE_LOG(LogTemp, Log, TEXT("[VacantPoint::RunTest()] %s (checking location %s)| TeamMember = %s -> EQSLocation = %s"),
					*CloneCharacter->GetName(),
					*EQSItemLocation.ToString(),
					*TeamMember->GetName(),
					*EQSLocation.ToString());
				
				const float Distance = (EQSItemLocation - EQSLocation).Length();
				
				UE_LOG(LogTemp, Log, TEXT("[VacantPoint::RunTest()] %s | Distance = %f, TeamMember = %s"),
					*CloneCharacter->GetName(),
					Distance,
					*TeamMember->GetName());

				// If TeamMember uses EQSLocation that is too close to the location
				// this npc is considering, don't pass this point
				if (Distance <= 50.0f)
				{
					CurrentPointPassed = false;
					UE_LOG(LogTemp, Log, TEXT("[VacantPoint::RunTest()] %s | point hasn't passed for TeamMember = %s"),
						*CloneCharacter->GetName(),
						*TeamMember->GetName());
					break;
				}
			}

			if (CurrentPointPassed)
				ItrEQSPoint.ForceItemState(EEnvItemStatus::Passed);
			else
				ItrEQSPoint.ForceItemState(EEnvItemStatus::Failed);
			
		}
	}
	
}

FText UEnvQueryTest_VacantPoint::GetDescriptionTitle() const
{
	return FText::FromString(FString::Printf(TEXT("VacantPoint")));
}

FText UEnvQueryTest_VacantPoint::GetDescriptionDetails() const
{
	return FText::FromString(FString::Printf(TEXT("Todo description")));
}