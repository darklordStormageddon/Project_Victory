#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MyGameInstanceSubsystem.generated.h"

class UMyGameInstance;

/**
 * 세션 검색 및 생성 담당 Subsystem
 * - GameInstance로부터 설정값을 받아 세션 생성
 * - 세션 검색, 참가, 파괴 등의 멀티플레이 로직 관리
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
};

UCLASS()
class TEAMSPACEPROJECT_API UMyGameInstanceSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // === 세션 관리 ===
    UFUNCTION(BlueprintCallable, Category = "Multiplayer|Session")
    void HostServer(bool IsPublic);

    UFUNCTION(BlueprintCallable, Category = "Multiplayer|Session")
    void FindSessions();

    UFUNCTION(BlueprintCallable, Category = "Multiplayer|Session")
    void RefreshSessions();

    UFUNCTION(BlueprintCallable, Category = "Multiplayer|Session")
    void JoinServer(const FString& Address);

    // === 기타 ===
    UFUNCTION(BlueprintCallable, Category = "Multiplayer|Level")
    void LoadLevelStreaming(FName LevelName);

    UFUNCTION(BlueprintCallable, Category = "Multiplayer|GameState")
    void SetGameStarted(bool IsGameStarted);

    // === 검색 결과 ===
    UPROPERTY(BlueprintReadOnly, Category = "Multiplayer|Session")
    TArray<FServerData> ServerNames;

private:
    // === 내부 콜백 ===
    void OnCreateSessionComplete(FName SessionName, bool bSuccess);
    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnDestroySessionComplete(FName SessionName, bool bSuccess);

    // === 온라인 세션 ===
    IOnlineSessionPtr SessionInterface;
    TSharedPtr<FOnlineSessionSearch> SessionSearch;

    // === 세션 생성 요청 상태 ===
    bool PendingIsPublic = true;

    // === 게임 인스턴스 참조 ===
    UPROPERTY()
    UMyGameInstance* GI = nullptr;

    // === 게임 고유 식별 태그 ===
    FString GameUniqueTag = TEXT("SpaceMiningSimulator");
};
