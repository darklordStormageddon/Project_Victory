// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Attachment_Base.generated.h"

class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttachmentDetached);

UCLASS()
class AAttachment_Base : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AAttachment_Base();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComp;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
	TArray<TSubclassOf<AAttachment_Base>> Attachment_Types;

	UPROPERTY(BlueprintAssignable)
	FOnAttachmentDetached OnDetached;

	UPROPERTY(BlueprintReadWrite, Category = "Health")
	bool bCanDamage;

public:
	UFUNCTION(BlueprintCallable, Category = "Chaos")
	void Broadcast_Attachment_Detachment();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Health")
	void Damage_Attachment(float Damage);
};
