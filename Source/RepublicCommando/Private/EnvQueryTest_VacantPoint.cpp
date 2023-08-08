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
	    // UE_LOG(LogTemp, Display, TEXT("[VacantPoint::RunTest()] Run Test, Querier is not valid"));
    	return;
    }
	
	// UE_LOG(LogTemp, Display, TEXT("[VacantPoint::RunTest()] Run Test, Context name = %s"), *(Querier->GetName()));

	// Get Owner of this EQS query
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr)
	{
		// UE_LOG(LogTemp, Display, TEXT("[VacantPoint::RunTest()] QueryOwner is nullptr"));
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
	
	
	TArray<FVector> ContextLocations;
	if (!QueryInstance.PrepareContext(Querier, ContextLocations))
	{
		UE_LOG(LogTemp, Warning, TEXT("[VacantPoint::RunTest()] QueryInstance.PrepareContext() returned false"));
		return;
	}

	
	FScopeLock Lock(Guard.Get());
	
	for (FEnvQueryInstance::ItemIterator ItrEQSPoint(this, QueryInstance); ItrEQSPoint; ++ItrEQSPoint)
	{
		const FVector EQSItemLocation = GetItemLocation(QueryInstance, ItrEQSPoint.GetIndex());
		if (CloneCharacter->TeamMembers.IsEmpty())
		{
			// UE_LOG(LogTemp, Display, TEXT("[VacantPoint::RunTest()] %s : Team is Empty | point has passed %s"),
			// 	*CloneCharacter->GetName(),
			// 	*EQSItemLocation.ToString());
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
			// UE_LOG(LogTemp, Log, TEXT("[VacantPoint::RunTest()] %s (checking location %s)| TeamMember = %s -> EQSLocation = %s"),
			// 	*CloneCharacter->GetName(),
			// 	*EQSItemLocation.ToString(),
			// 	*TeamMember->GetName(),
			// 	*TeamMemberEQSLocation.ToString());
			
			const float DistanceToOtherCloneEQS = (EQSItemLocation - TeamMemberEQSLocation).Length();
			const float DistanceToOtherClonePos = (EQSItemLocation - TeamMember->GetActorLocation()).Length();
			
			// UE_LOG(LogTemp, Log, TEXT("[VacantPoint::RunTest()] %s | Distance = %f, TeamMember = %s"),
			// 	*CloneCharacter->GetName(),
			// 	DistanceToOtherCloneEQS,
			// 	*TeamMember->GetName());

			// If TeamMember uses EQSLocation that is too close to the location
			// this npc is considering, don't pass this point
			if (DistanceToOtherCloneEQS <= 50.0f || DistanceToOtherClonePos <= 150.0f)
			{
				CurrentPointPassed = false;
				// UE_LOG(LogTemp, Log, TEXT("[VacantPoint::RunTest()] %s | point hasn't passed for TeamMember = %s : either of Distances are too small"),
				// 	*CloneCharacter->GetName(),
				// 	*TeamMember->GetName());
				break;
			}
		}

		// // TODO: удалить 
		// uint32 ThreadId = FPlatformTLS::GetCurrentThreadId();
		// FString ThreadName = FThreadManager::Get().GetThreadName(ThreadId);
		// UE_LOG(LogTemp, Log, TEXT("VacantPoint::ThreadName = %s"), *ThreadName);
		

		if (CurrentPointPassed)
		{
			ItrEQSPoint.ForceItemState(EEnvItemStatus::Passed);
			// UE_LOG(LogTemp, Log, TEXT("[VacantPoint::RunTest()] %s | point %s has passed"),
			// 		*CloneCharacter->GetName(),
			// 		*EQSItemLocation.ToString());
		}
		else
		{
			ItrEQSPoint.ForceItemState(EEnvItemStatus::Failed);
			// UE_LOG(LogTemp, Log, TEXT("[VacantPoint::RunTest()] %s | point %s hasn't passed"),
			// 		*CloneCharacter->GetName(),
			// 		*EQSItemLocation.ToString());
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