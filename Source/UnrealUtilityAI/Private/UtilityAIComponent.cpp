// Copyright 2018 Roberto De Ioris

#include "UtilityAIComponent.h"


// Sets default values for this component's properties
UUtilityAIComponent::UUtilityAIComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UUtilityAIComponent::BeginPlay()
{
	Super::BeginPlay();

	LastAction = nullptr;
	LastPawn = nullptr;

	AAIController* Controller = Cast<AAIController>(GetOwner());
	if (!Controller)
		return;

	// instantiate actions
	for (TSubclassOf<UUtilityAIAction> ActionClass : Actions)
	{
		UUtilityAIAction* Action = NewObject<UUtilityAIAction>(Controller, ActionClass);
		InstancedActions.Add(Action);
		OnUtilityAIActionSpawned.Broadcast(Action);
	}

}


// Called every frame
void UUtilityAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AAIController* Controller = Cast<AAIController>(GetOwner());
	if (!Controller)
		return;

	APawn* Pawn = Controller->GetPawn();

	UUtilityAIAction* BestAction = nullptr;

	for (UUtilityAIAction* Action : InstancedActions)
	{
		if (!Action->CanRun(Controller, Pawn))
			continue;
		Action->LastScore = Action->Score(Controller, Pawn);
		if (bIgnoreZeroScore && Action->LastScore == 0)
			continue;

		if (!BestAction || BestAction->LastScore < Action->LastScore)
		{
			BestAction = Action;
		}
	}

	if (BestAction)
	{
		if (LastAction != BestAction)
		{
			OnUtilityAIActionChanged.Broadcast(BestAction, LastAction);
			if (LastAction)
			{
				LastAction->Exit(Controller, LastPawn);
			}
			BestAction->Enter(Controller, Pawn);
		}
		BestAction->Tick(DeltaTime, Controller, Pawn);
		LastAction = BestAction;
		LastPawn = Pawn;
	}
	else
	{
		OnUtilityAIActionNotAvailable.Broadcast();
		if (LastAction)
		{
			OnUtilityAIActionChanged.Broadcast(nullptr, LastAction);
			LastAction->Exit(Controller, LastPawn);
			LastAction = nullptr;
			LastPawn = nullptr;
		}
	}
}

TArray<UUtilityAIAction*> UUtilityAIComponent::GetActionInstances() const
{
	return InstancedActions.Array();
}

