// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JHS/UI/UIBase.h"
#include "UIManager.generated.h"

class UWidgetComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UUIManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUIManager(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION(BlueprintCallable, Category = "UIManager")
	UUIBase* OpenUI(E_UI_TYPE UIType);

	UFUNCTION(BlueprintCallable, Category = "UIManager")
	UUIBase* OpenUIInWorld(E_UI_TYPE UIType, AActor* OwnerActor, FVector RelativeLocation = FVector(0, 0, 100), float Scale = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "UIManager")
	UUIBase* CloseUI(E_UI_TYPE UIType);

	UFUNCTION(BlueprintCallable, Category = "UIManager")
	void CloseAllUI();

	UFUNCTION(BlueprintCallable, Category = "UIManager")
	UUIBase* GetUI(E_UI_TYPE UIType) const;

private:
	UUIBase* OpenUIInternal(E_UI_TYPE UIType);

	template<typename T>
	T* LoadUI(E_UI_TYPE UIType)
	{
		return Cast<T>(LoadUIInternal(UIType));
	}

private:
	UUIBase* LoadUIInternal(E_UI_TYPE UIType);

	UUIBase* InstantiateUI(E_UI_TYPE UIType);

	FString GetUIPath(E_UI_TYPE UIType) const;

private:
	UPROPERTY()
	TMap<E_UI_TYPE, TObjectPtr<UUIBase>> _loadedUIDict;

	UPROPERTY()
	TMap<E_UI_TYPE, TObjectPtr<UWidgetComponent>> _worldSpaceUIComponents;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Manager|Settings", meta = (AllowPrivateAccess = "true"))
	TMap<E_UI_TYPE, TSoftClassPtr<UUIBase>> _uiClassMap;
};
