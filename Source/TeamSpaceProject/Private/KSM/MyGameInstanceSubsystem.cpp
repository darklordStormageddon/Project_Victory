#include "KSM/MyGameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "MyGameInstance.h"

#define SEARCH_PRESENCE FName(TEXT("PRESENCE"))

void UMyGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 안전한 초기화: OnlineSubsystem 완전히 준비된 후 가져오기
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (!OSS)
    {
        UE_LOG(LogTemp, Error, TEXT("OnlineSubsystem not found during Initialize."));
        return;
    }

    SessionInterface = OSS->GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SessionInterface invalid during Initialize."));
        return;
    }

    // 델리게이트 등록
    SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMyGameInstanceSubsystem::OnCreateSessionComplete);
    SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMyGameInstanceSubsystem::OnDestroySessionComplete);
    SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMyGameInstanceSubsystem::OnFindSessionsComplete);

    UE_LOG(LogTemp, Log, TEXT("UMyGameInstanceSubsystem initialized and delegates bound."));
}

// =============================
// Host
// =============================
void UMyGameInstanceSubsystem::HostServer(bool IsPublic)
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SessionInterface invalid on HostServer."));
        return;
    }

    // 게임 인스턴스 캐시
    GI = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (!GI)
    {
        UE_LOG(LogTemp, Error, TEXT("GameInstance not found."));
        return;
    }

    // 기존 세션이 있다면 먼저 삭제
    FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (ExistingSession)
    {
        UE_LOG(LogTemp, Warning, TEXT("Existing session found. Destroying before creating new one."));
        PendingIsPublic = IsPublic;
        SessionInterface->DestroySession(NAME_GameSession);
        return;
    }

    // 새로운 세션 설정
    FOnlineSessionSettings Settings;
    Settings.NumPublicConnections = 4;
    Settings.bIsLANMatch = true;
    Settings.bUsesPresence = true;
    Settings.bAllowJoinInProgress = true;
    Settings.bAllowJoinViaPresence = true;
    Settings.bShouldAdvertise = true;

    // 메타데이터 설정
    Settings.Set(FName("GameUniqueTag"), GameUniqueTag, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    Settings.Set(TEXT("Public"), IsPublic, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    Settings.Set(TEXT("SessionName"), GI->RoomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    UE_LOG(LogTemp, Log, TEXT("Creating session... [Public: %s] [Room: %s]"),
        IsPublic ? TEXT("true") : TEXT("false"), *GI->RoomName);

    SessionInterface->CreateSession(0, NAME_GameSession, Settings);
}

void UMyGameInstanceSubsystem::OnCreateSessionComplete(FName SessionName, bool bSuccess)
{
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("Session creation failed: %s"), *SessionName.ToString());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Session created successfully: %s"), *SessionName.ToString());

    // 서버 전환 - 타이머를 사용하여 안전하게 지연 후 이동
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World is null, cannot travel to Lobby."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Scheduling server travel to Lobby..."));

    FTimerHandle TravelTimerHandle;
    World->GetTimerManager().SetTimer(TravelTimerHandle, [World]()
        {
            if (World && World->IsValidLowLevel())
            {
                UE_LOG(LogTemp, Log, TEXT("Server traveling to Lobby..."));
                World->ServerTravel(TEXT("/Game/Imports/Maps/Lobby?listen"));
            }
        }, 0.5f, false); // 0.5초 지연으로 세션이 완전히 등록되도록 대기
}

void UMyGameInstanceSubsystem::OnDestroySessionComplete(FName SessionName, bool bSuccess)
{
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to destroy existing session: %s"), *SessionName.ToString());
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Previous session destroyed. Recreating..."));

    // 세션 삭제 후 약간의 지연을 두고 새 세션 생성
    UWorld* World = GetWorld();
    if (World)
    {
        FTimerHandle RecreateTimerHandle;
        World->GetTimerManager().SetTimer(RecreateTimerHandle, [this]()
            {
                HostServer(PendingIsPublic);
            }, 0.3f, false);
    }
    else
    {
        HostServer(PendingIsPublic);
    }
}

// =============================
// Find / Refresh
// =============================
void UMyGameInstanceSubsystem::FindSessions()
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SessionInterface invalid on FindSessions."));
        return;
    }

    ServerNames.Empty();
    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->bIsLanQuery = true;
    SessionSearch->MaxSearchResults = 100;
    SessionSearch->PingBucketSize = 50;

    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    UE_LOG(LogTemp, Log, TEXT("Searching for sessions..."));
    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UMyGameInstanceSubsystem::RefreshSessions()
{
    UE_LOG(LogTemp, Log, TEXT("Refreshing sessions..."));
    FindSessions();
}

void UMyGameInstanceSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (!bWasSuccessful || !SessionSearch.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("FindSessions failed or invalid search object."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("FindSessions complete: %d results"), SessionSearch->SearchResults.Num());
    ServerNames.Empty();

    for (const auto& Result : SessionSearch->SearchResults)
    {
        FString FoundTag;
        if (!Result.Session.SessionSettings.Get(FName("GameUniqueTag"), FoundTag))
            continue;
        UE_LOG(LogTemp, Error, TEXT("게임태그 못찾음."));
        if (FoundTag != GameUniqueTag)
            continue;
        UE_LOG(LogTemp, Error, TEXT("게임태그 찾음."));
        FServerData Data;
        Result.Session.SessionSettings.Get(FName("SessionName"), Data.Name);
        Result.Session.SessionSettings.Get(FName("Public"), Data.Accessibility);

        Data.HostUserName = Result.Session.OwningUserName;
        int32 MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
        Data.CurrentPlayers = MaxPlayers - Result.Session.NumOpenPublicConnections;
        UE_LOG(LogTemp, Error, TEXT("정보 저장 완."));
        if (!Data.Accessibility && GI)
            Data.Password = GI->Password.ToString();

        ServerNames.Add(Data);

        UE_LOG(LogTemp, Log, TEXT("Session Found: %s | Host: %s | Players: %d/%d"),
            *Data.Name, *Data.HostUserName, Data.CurrentPlayers, MaxPlayers);
    }

    if (ServerNames.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No active sessions found."));
    }
}

// =============================
// Join / Level
// =============================
void UMyGameInstanceSubsystem::JoinServer(const FString& Address)
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        UE_LOG(LogTemp, Log, TEXT("Joining server at %s"), *Address);
        PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
    }
}

void UMyGameInstanceSubsystem::LoadLevelStreaming(FName LevelName)
{
    if (UWorld* World = GetWorld())
    {
        FLatentActionInfo LatentInfo;
        LatentInfo.CallbackTarget = this;
        UGameplayStatics::LoadStreamLevel(World, LevelName, true, true, LatentInfo);
    }
}

void UMyGameInstanceSubsystem::SetGameStarted(bool IsGameStarted)
{
    if (GI)
        GI->bIsGameStarted = IsGameStarted;
}