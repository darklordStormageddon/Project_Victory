// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

class UMyGameInstanceSubsystem;
/**
 * 
 */


UCLASS()
class UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
    virtual void Init() override;

    // Listen Server 생성
    UFUNCTION(BlueprintCallable)
    void HostServer();

    // 서버 검색
    UFUNCTION(BlueprintCallable)
    void FindServers();

    UFUNCTION(BlueprintCallable)
    void RefreshServers();

    // 서버 참가
    UFUNCTION(BlueprintCallable)
    void JoinServer(FString ServerAddress);

    UPROPERTY(BlueprintReadOnly)
    UMyGameInstanceSubsystem* GIS;

    UPROPERTY(BlueprintReadWrite)
    FText Password;

    bool bIsGameStarted;

    //게임 접근성
    UPROPERTY(BlueprintReadWrite)
    bool IsPublic;
    

    UPROPERTY(BlueprintReadWrite)
    FString RoomName;
};