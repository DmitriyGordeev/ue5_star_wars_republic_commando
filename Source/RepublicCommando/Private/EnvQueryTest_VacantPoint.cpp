// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvQueryTest_VacantPoint.h"

#include "BaseCloneCharacter.h"
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
	    UE_LOG(LogTemp, Display, TEXT("Run Test, Querier is not valid"));
    	return;
    }
	
	UE_LOG(LogTemp, Display, TEXT("Run Test, Context name = %s"), *(Querier->GetName()));

	// Get Owner of this EQS query
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr)
	{
		UE_LOG(LogTemp, Display, TEXT("QueryOwner is nullptr"));
		return;
	}

	// If it's not ABaseCloneCharacter we return
	ABaseCloneCharacter* CloneCharacter = Cast<ABaseCloneCharacter>(QueryOwner);
	if (CloneCharacter)
	{
		UE_LOG(LogTemp, Display, TEXT("Character.GetName() = %s"), *CloneCharacter->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("CloneCharacter is not valid"));
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
		UE_LOG(LogTemp, Warning, TEXT("QueryInstance.PrepareContext() returned false"));
		return;
	}

	for (FEnvQueryInstance::ItemIterator ItrEQSPoint(this, QueryInstance); ItrEQSPoint; ++ItrEQSPoint)
	{
		const FVector EQSItemLocation = GetItemLocation(QueryInstance, ItrEQSPoint.GetIndex());
		if (CloneCharacter->TeamMembers.IsEmpty())
		{
			UE_LOG(LogTemp, Display, TEXT("Team is Empty"));
			
			// TODO: надо убрать хардкод из Score:
			ItrEQSPoint.SetScore(TestPurpose, FilterType, 1000, MinThresholdValue, MaxThresholdValue);
		}
		else
		{
			for(auto it = CloneCharacter->TeamMembers.begin();
				it != CloneCharacter->TeamMembers.end();
				++it)
			{
				ABaseCloneCharacter* TeamMember = *it;
				if (TeamMember == nullptr)
					continue;

				// variant because team member might not have chosen eqs point yet,
				// hence it's FVector or bool
				FVariant SelectedEQS = TeamMember->GetSelectedEQSLocation();

				// if SelectedEQS is of type FVector we check Distance
				if (SelectedEQS.GetType() == EVariantTypes::Vector)
				{
					FVector SelectedEQSLocation = SelectedEQS.GetValue<FVector>();
					const float Distance = (EQSItemLocation - SelectedEQSLocation).Length();

					UE_LOG(LogTemp, Display,
						TEXT("Item = %s, TargetLoc = %s, Distance = %f"),
						*EQSItemLocation.ToString(), *SelectedEQSLocation.ToString(), Distance);
						
					ItrEQSPoint.SetScore(TestPurpose, FilterType, Distance, MinThresholdValue, MaxThresholdValue);
				}
				else
				{
					// TODO: надо убрать хардкод из Score:
					ItrEQSPoint.SetScore(TestPurpose, FilterType, 1000, MinThresholdValue, MaxThresholdValue);
				}
			}
			
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