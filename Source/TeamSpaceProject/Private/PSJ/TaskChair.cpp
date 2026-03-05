


#include "PSJ/TaskChair.h"
#include "JHS/UI/UIBase.h"
#include "PSJ/TaskPawnBase.h"
#include "PSJ_Spaceship.h"
#include "PSJ_ToolBase.h"
#include "Net/UnrealNetwork.h"
#include "PSJ_Character.h" 
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h" 
#include "GameFramework/Controller.h" 
#include "GameFramework/PlayerController.h"

ATaskChair::ATaskChair()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	bReplicates = true;
}

void ATaskChair::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		AActor* ManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASolarWindManager::StaticClass());
		if (ASolarWindManager* WindManager = Cast<ASolarWindManager>(ManagerActor))
		{
			WindManager->OnSolarWindImpact.AddDynamic(this, &ATaskChair::HandleSolarWindEvent);
		}
	}
}

void ATaskChair::SetTargetTaskPawn(ATaskPawnBase* NewTaskPawn)
{
    TargetTaskPawn = NewTaskPawn;
}

void ATaskChair::HandleSolarWindEvent()
{

	if (!HasAuthority()) return;


	if (bIsMalfunctioning) return;


	float DiceRoll = FMath::FRand(); 

	if (DiceRoll <= MalfunctionProbability)
	{
		StartMalfunction();
	}
}

void ATaskChair::StartMalfunction()
{
	if (!HasAuthority()) return; 


	if (bIsMalfunctioning)
	{
		CurrentMalfunctionTimer = MalfunctionDuration;
		return;
	}


	ReceiveForceEjectRequest();


	bIsMalfunctioning = true;
	CurrentMalfunctionTimer = MalfunctionDuration;
	RepairingCharacters.Empty(); 


	OnRep_IsMalfunctioning();

}

void ATaskChair::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATaskChair, TargetTaskPawn);
	DOREPLIFETIME(ATaskChair, bIsMalfunctioning); 
    DOREPLIFETIME(ATaskChair, bIsOccupied);
	DOREPLIFETIME(ATaskChair, RepairingCharacters);
}

void ATaskChair::OnInteractEnter(int32 CallerPlayerId, TObjectPtr<UUIBase> OpenedUI)
{

	if (bIsMalfunctioning)
	{
		return;
	}

	Super::OnInteractEnter(CallerPlayerId, OpenedUI);

	if (TargetTaskPawn == nullptr)
	{
		return;
	}

	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	APSJ_Character* MyChar = Cast<APSJ_Character>(PlayerPawn);

	if (!MyChar) return;

	MyChar->Server_RequestBoarding(TargetTaskPawn);
}

void ATaskChair::OnInteractExit(int32 CallerPlayerId, TObjectPtr<UUIBase> ClosedUI)
{
	Super::OnInteractExit(CallerPlayerId, ClosedUI);
}

void ATaskChair::ReceiveForceEjectRequest()
{
    if (!TargetTaskPawn || TargetTaskPawn->GetController() == nullptr) return;

    TargetTaskPawn->DisembarkCharacter();
}

void ATaskChair::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (HasAuthority() && bIsMalfunctioning)
    {
        float TotalSpeed = 1.0f;

        for (APSJ_Character* Mechanic : RepairingCharacters)
        {
            if (Mechanic && Mechanic->EquippedTool)
            {
                TotalSpeed += Mechanic->EquippedTool->GetCurrentRepairSpeed();
            }
            else
            {
                TotalSpeed += 0.5f;
            }
        }


        CurrentMalfunctionTimer -= (DeltaTime * TotalSpeed);


        if (CurrentMalfunctionTimer <= 0.0f)
        {
            bIsMalfunctioning = false;
            CurrentMalfunctionTimer = 0.0f;
            RepairingCharacters.Empty();

            OnRep_IsMalfunctioning(); 
        }
    }


    if (GetWorld()->IsNetMode(NM_Client) || GetWorld()->IsPlayInEditor())
    {
        FVector ActorLoc = GetActorLocation();

        FVector StatusLoc = ActorLoc + FVector(0, 0, 30.0f); 

        if (bIsMalfunctioning)
        {

            FString StatusMsg = FString::Printf(TEXT("MALFUNCTION!"), CurrentMalfunctionTimer);
            DrawDebugString(GetWorld(), StatusLoc, StatusMsg, nullptr, FColor::Red, 0.0f);
        }
        if (RepairingCharacters.Num() > 0)
        {
            FVector RepairLoc = ActorLoc + FVector(0, 0, 50.0f);
            FString RepairMsg = FString::Printf(TEXT("Repairing... (%d People)"), RepairingCharacters.Num());
            DrawDebugString(GetWorld(), RepairLoc, RepairMsg, nullptr, FColor::Yellow, 0.0f);
        }
    }

}


void ATaskChair::AddRepairer(APSJ_Character* Mechanic)
{
    if (Mechanic && !RepairingCharacters.Contains(Mechanic))
    {
        RepairingCharacters.Add(Mechanic);
    }
}

void ATaskChair::RemoveRepairer(APSJ_Character* Mechanic)
{
    if (Mechanic)
    {
        RepairingCharacters.Remove(Mechanic);
    }
}

void ATaskChair::OnRep_IsOccupied()
{
	TArray<USphereComponent*> Spheres;
	GetComponents<USphereComponent>(Spheres);

	for (USphereComponent* Sphere : Spheres)
	{

		if (Sphere->GetName().Contains(TEXT("CollisionComponent")))
		{
			if (bIsOccupied)
			{

				Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
			else
			{

				Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			}
		}
	}
}


void ATaskChair::OnRep_IsMalfunctioning()
{
	{

		TArray<USphereComponent*> Spheres;
		GetComponents<USphereComponent>(Spheres);

		for (USphereComponent* Sphere : Spheres)
		{
			if (Sphere->GetName().Contains(TEXT("CollisionComponent")))
			{
				if (bIsOccupied)
				{
					Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
				else
				{
					Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				}
			}
		}
	}

    if (bIsMalfunctioning)
    {
    }
    else
    {
    }
}