// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MyGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionListUpdated);

class UMyGameInstanceSubsystem;
/**
 * 
 */

USTRUCT(BlueprintType)
struct FServerData
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Category = "Server Info")
    FString Name;

    UPROPERTY(BlueprintReadOnly, Category = "Server Info")
    int CurrentPlayers = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Server Info")
    FString HostUserName;

    UPROPERTY(BlueprintReadOnly, Category = "Server Info")
    bool Accessibility = true;

    UPROPERTY(BlueprintReadOnly, Category = "Server Info")
    FString Password;

    UPROPERTY(BlueprintReadOnly, Category = "Server Info")
    int32 SearchResultIndex = -1;
};

UCLASS()
class UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
    virtual void Init() override;

public:

	//방생성
	UFUNCTION(BlueprintCallable, Exec)
	void Host(FString ServerName);

	UFUNCTION(BlueprintCallable, Exec)
	void Join(int Index);

	UFUNCTION(BlueprintCallable, Exec)
	void RefreshServerList();

	UFUNCTION(BlueprintCallable, Exec)
	void StartSession();

private:
	void OnCreateSessionComplete(FName InSessionName, bool IsSuccess);
	void OnDestroySessionComplete(FName InSessionName, bool IsSuccess);
	void OnFindSessionComplete(bool IsSuccess);
	void OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type InResult);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	void CreateSession();

	FString GameUniqueTag;
    IOnlineSessionPtr SessionInterface; //세션 생성할때 쓰는 인터페이스
    TSharedPtr<FOnlineSessionSearch> SessionSearch;

public:
    UPROPERTY(BlueprintReadWrite)
    FString Password;

	UPROPERTY(BlueprintReadWrite)
    bool bIsGameStarted;

    //게임 접근성
    UPROPERTY(BlueprintReadWrite)
    bool bIsPublic;

    // 블루프린트에서 이벤트 바인딩 가능
    UPROPERTY(BlueprintAssignable, Category = "Session")
    FOnSessionListUpdated OnSessionListUpdated;

    UPROPERTY(BlueprintReadWrite)
    FString RoomName;

    UPROPERTY(BlueprintReadWrite)
    FString SearchName;

    UPROPERTY(BlueprintReadOnly)
    TArray<FServerData> ServerNames;
};