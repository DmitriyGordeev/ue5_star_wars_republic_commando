// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvQueryTest_VacantPoint.h"
#include "BaseCloneCharacter.h"
#include "HAL/ThreadManager.h"
// #include "Misc/ScopeLock.h"
#include "ParticleHelper.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"


UEnvQueryTest_VacantPoint::UEnvQueryTest_VacantPoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Setup context object
	Querier = UEnvQueryContext::StaticClass();
	Cost = EEnvTestCost::Low;
	Guard = MakeShared<FCriticalSection>();

	// Specify item type (Actor, Vector, todo: изменить?)
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
}


void UEnvQueryTest_VacantPoint::RunTest(FEnvQueryInstance& QueryInstance) const
{
    if (!Querier)
    {
    	return;
    }
	
	// Get Owner of this EQS query
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr)
	{
		return;
	}

	// If it's not ABaseCloneCharacter we return
	ABaseCloneCharacter* CloneCharacter = Cast<ABaseCloneCharacter>(QueryOwner);
	if (!CloneCharacter)
		return;
	
	TArray<FVector> ContextLocations;
	if (!QueryInstance.PrepareContext(Querier, ContextLocations))
		return;
	
	FScopeLock Lock(Guard.Get());
	
	for (FEnvQueryInstance::ItemIterator ItrEQSPoint(this, QueryInstance); ItrEQSPoint; ++ItrEQSPoint)
	{
		const FVector EQSItemLocation = GetItemLocation(QueryInstance, ItrEQSPoint.GetIndex());
		if (CloneCharacter->TeamMembers.IsEmpty())
		{
			ItrEQSPoint.ForceItemState(EEnvItemStatus::Passed);
			break;
		}
		
		bool CurrentPointPassed = true;
		
		for(auto it = CloneCharacter->TeamMembers.begin();
			it != CloneCharacter->TeamMembers.end();
			++it)
		{
			const ABaseCloneCharacter* TeamMember = *it;
			if (TeamMember == nullptr)
				continue;
			
			FVector TeamMemberEQSLocation = TeamMember->GetSelectedEQSLocation();
			const float DistanceToOtherCloneEQS = (EQSItemLocation - TeamMemberEQSLocation).Length();
			const float DistanceToOtherClonePos = (EQSItemLocation - TeamMember->GetActorLocation()).Length();

			// If TeamMember uses EQSLocation that is too close to the location
			// this npc is considering, don't pass this point
			if (DistanceToOtherCloneEQS <= 200.0f || DistanceToOtherClonePos <= 300.0f)
			{
				CurrentPointPassed = false;
				break;
			}
		}

		if (CurrentPointPassed)
		{
			ItrEQSPoint.ForceItemState(EEnvItemStatus::Passed);
		}
		else
		{
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
	return FText::FromString(FString::Printf(TEXT("Prevents from picking EQS location if other team-member has picked it")));
}