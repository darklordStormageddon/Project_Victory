// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MyGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionListUpdated);

USTRUCT(BlueprintType)
struct FServerData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	FString Name;
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentPlayers;
	UPROPERTY(BlueprintReadWrite)
	FString HostUserName;
	UPROPERTY(BlueprintReadWrite)
	FString Password;
	UPROPERTY(BlueprintReadWrite)
	bool Accessibility;
	UPROPERTY(BlueprintReadWrite)
	int32 SearchResultIndex;
};

UCLASS()
class UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UMyGameInstance();

protected:
	virtual void Init() override;
public:

	//방생성
	UFUNCTION(BlueprintCallable, Exec)
	void Host(FString ServerName);
	UFUNCTION(BlueprintCallable, Exec)
	void Join(int32 Index);
	UFUNCTION(BlueprintCallable, Exec)
	void RefreshServerList();

private:
	void OnCreateSessioncomplete(FName InSessionName, bool IsSuccess);
	void OnDestroySessioncomplete(FName InSessionName, bool IsSuccess);
	void OnFindSessioncomplete(bool IsSuccess);
	void OnJoinSessioncomplete(FName InSessionName, EOnJoinSessionCompleteResult::Type InResult);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	
	bool bPendingTravel = false;

	void CreateSession();


public:
	UFUNCTION(BlueprintCallable)
	void StartSession();

	IOnlineSessionPtr GetSessionInterface() const { return SessionInterface; }
private:

	//내가 생성한 방(세션)이름
	FString DesiredServerName;
	IOnlineSessionPtr SessionInterface; //세션 생성할때 쓰는 인터페이스

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FString GameUniqueTag = "Project_Victory";

	bool bRecreateAfterDestroy;

public:
	UPROPERTY(BlueprintReadWrite)
	TArray<FServerData> ServerNames;

	UPROPERTY(BlueprintReadWrite)
	FString Password;

	UPROPERTY(BlueprintReadWrite)
	bool bIsGameStarted;

	UPROPERTY(BlueprintReadWrite)
	FString NickName;

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
};