// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include <Kismet/GameplayStatics.h>
#include "Engine/NetConnection.h"
#include "Net/Core/Connection/NetCloseResult.h"
#include <Online/OnlineSessionNames.h>
#include "GameFramework/PlayerState.h"

//초보채널,중수채널
const static FName SESSION_NAME = TEXT("GameSession"); //채널명
const static FName SESSION_SETTINGS_KEY = TEXT("FREE");//게임모드

// Steam 호환 세션 키 정의
static const FName SETTING_SERVER_NAME = FName(TEXT("SERVER_NAME_KEY"));
static const FName SETTING_GAME_TAG = FName(TEXT("GAME_TAG_KEY"));
static const FName SETTING_IS_PUBLIC = FName(TEXT("IS_PUBLIC_KEY"));
static const FName SETTING_ROOM_NAME = FName(TEXT("ROOM_NAME_KEY"));
static const FName SETTING_PASSWORD = FName(TEXT("PASSWORD_KEY"));

UMyGameInstance::UMyGameInstance()
{
}


void UMyGameInstance::Init()
{
	Super::Init();

	GameUniqueTag = "Project_Victory";
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (OSS)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("OSS : %s is Avaliable."), *OSS->GetSubsystemName().ToString());

		SessionInterface = OSS->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnCreateSessioncomplete);

			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnDestroySessioncomplete);

			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnFindSessioncomplete);

			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnJoinSessioncomplete);

			SessionInterface->OnStartSessionCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnStartSessionComplete);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not found subsystem."));
	}

	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UMyGameInstance::OnNetworkFailure);
	}
}



void UMyGameInstance::Host(FString ServerName)
{
	DesiredServerName = ServerName;
	if (SessionInterface.IsValid())
	{
		auto AlreadyExsistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
		if (AlreadyExsistingSession)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s is already exsist. re-createSession."), *SESSION_NAME.ToString());
			SessionInterface->DestroySession(SESSION_NAME);
			bRecreateAfterDestroy = true;
		}
		else
		{
			CreateSession();
		}
	}
}

void UMyGameInstance::CreateSession()
{
	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings SessionSettings;

		if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
			SessionSettings.bIsLANMatch = true;
		else
			SessionSettings.bIsLANMatch = false;

		SessionSettings.NumPublicConnections = 4;
		SessionSettings.bUsesPresence = true;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bAllowJoinInProgress = true;
		SessionSettings.bAllowJoinViaPresence = true;
		SessionSettings.bUseLobbiesIfAvailable = true;

		//빌드 체크 비활성화
		SessionSettings.bAntiCheatProtected = false;
		SessionSettings.BuildUniqueId = 0;

		//나머지 설정
		SessionSettings.Set(FName("SERVER_NAME_KEY"), DesiredServerName,
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		SessionSettings.Set(FName("GAME_TAG_KEY"), GameUniqueTag,
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		SessionSettings.Set(FName("IS_PUBLIC_KEY"), bIsPublic,
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		SessionSettings.Set(FName("ROOM_NAME_KEY"), RoomName,
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		if (!Password.IsEmpty())
		{
			SessionSettings.Set(FName("PASSWORD_KEY"), Password,
				EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		}

		SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
	}
}

void UMyGameInstance::RefreshServerList()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Finding Session"));
		//세션 100개 최대 찾아온다.
		SessionSearch->MaxSearchResults = 100;

		// Steam을 사용할 때는 false, NULL 서브시스템일 때는 true
		if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
			SessionSearch->bIsLanQuery = true;
		else
			SessionSearch->bIsLanQuery = false;

		//SessionSearch->QuerySettings.Set(FName(TEXT("PRESENCESEARCH")), true, EOnlineComparisonOp::Equals);
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UMyGameInstance::Join(int32 Index)
{
	if (!SessionInterface.IsValid()) return;
	if (!SessionSearch.IsValid()) return;

	if (SessionSearch->SearchResults.Num() > (int32)Index)
		SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
	else
		UE_LOG(LogTemp, Warning, TEXT("Empty Session"));
}

void UMyGameInstance::StartSession()
{
	if (SessionInterface.IsValid())
		SessionInterface->StartSession(SESSION_NAME);
}

void UMyGameInstance::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("OnStartSessionComplete: %s, Success=%d"),
		*SessionName.ToString(), bWasSuccessful ? 1 : 0);

	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to start session!"));
		return;
	}

	// 세션 상태 확인
	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	if (Session)
	{
		UE_LOG(LogTemp, Warning, TEXT("Session State: %d (0=Pending, 1=Starting, 2=InProgress, 3=Ending, 4=Ended, 5=Destroying)"),
			(int32)Session->SessionState);
	}

	// RegisterPlayer
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->PlayerState)
	{
		TSharedPtr<const FUniqueNetId> UserId = PC->PlayerState->GetUniqueId().GetUniqueNetId();
		if (UserId.IsValid())
		{
			bool bRegistered = SessionInterface->RegisterPlayer(SessionName, *UserId, false);
			UE_LOG(LogTemp, Warning, TEXT("RegisterPlayer result: %d"), bRegistered ? 1 : 0);

			// 등록 후 세션 상태 다시 확인
			Session = SessionInterface->GetNamedSession(SessionName);
			if (Session)
			{
				UE_LOG(LogTemp, Warning, TEXT("After RegisterPlayer - State: %d, Players: %d"),
					(int32)Session->SessionState, Session->RegisteredPlayers.Num());

				// 세션을 InProgress로 강제 전환
				if (Session->SessionState == EOnlineSessionState::Pending)
				{
					UE_LOG(LogTemp, Warning, TEXT("Session still Pending! Trying to update session..."));

					// 세션 설정을 다시 업데이트하여 InProgress로 전환
					FOnlineSessionSettings UpdatedSettings = Session->SessionSettings;
					UpdatedSettings.bAllowJoinInProgress = true;

					SessionInterface->UpdateSession(SessionName, UpdatedSettings);
				}
			}
		}
	}
}

void UMyGameInstance::OnCreateSessioncomplete(FName InSessionName, bool IsSuccess)
{
	UE_LOG(LogTemp, Warning, TEXT("=== OnCreateSessionComplete ==="));
	UE_LOG(LogTemp, Warning, TEXT("SessionName: %s, Success: %d"), *InSessionName.ToString(), IsSuccess ? 1 : 0);

	if (!IsSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create session!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Session created successfully."));

	// World 확인
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("World is NULL!"));
		return;
	}

	// PlayerController 확인
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerController is NULL!"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("PlayerController found"));

	// PlayerState 확인
	if (!PC->PlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerState is NULL!"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("PlayerState found"));

	// UniqueNetId 확인
	TSharedPtr<const FUniqueNetId> UserId = PC->PlayerState->GetUniqueId().GetUniqueNetId();
	if (!UserId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UniqueNetId is INVALID!"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("UniqueNetId: %s"), *UserId->ToString());

	// RegisterPlayer 시도
	UE_LOG(LogTemp, Warning, TEXT("Calling RegisterPlayer..."));
	bool bRegistered = SessionInterface->RegisterPlayer(InSessionName, *UserId, false);
	UE_LOG(LogTemp, Warning, TEXT("RegisterPlayer returned: %d"), bRegistered ? 1 : 0);

	// 세션 상태 확인
	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(InSessionName);
	if (Session)
	{
		UE_LOG(LogTemp, Warning, TEXT("After RegisterPlayer:"));
		UE_LOG(LogTemp, Warning, TEXT("  SessionState: %d (0=Pending, 1=Starting, 2=InProgress, 3=Ending, 4=Ended, 5=Destroying)"),
			(int32)Session->SessionState);
		UE_LOG(LogTemp, Warning, TEXT("  RegisteredPlayers: %d"), Session->RegisteredPlayers.Num());

		// 등록된 플레이어 목록 출력
		for (int32 i = 0; i < Session->RegisteredPlayers.Num(); i++)
		{
			UE_LOG(LogTemp, Warning, TEXT("    Player %d: %s"), i, *Session->RegisteredPlayers[i]->ToString());
		}

		// UpdateSession 시도
		UE_LOG(LogTemp, Warning, TEXT("Calling UpdateSession..."));
		FOnlineSessionSettings UpdatedSettings = Session->SessionSettings;
		bool bUpdated = SessionInterface->UpdateSession(InSessionName, UpdatedSettings, true);
		UE_LOG(LogTemp, Warning, TEXT("UpdateSession returned: %d"), bUpdated ? 1 : 0);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Session not found after RegisterPlayer!"));
	}

	UE_LOG(LogTemp, Warning, TEXT("================================="));
}

void UMyGameInstance::OnDestroySessioncomplete(FName InSessionName, bool IsSuccess)
{
	if (IsSuccess && bRecreateAfterDestroy)
	{
		bRecreateAfterDestroy = false;
		CreateSession();
	}
}

void UMyGameInstance::OnFindSessioncomplete(bool IsSuccess)
{
	UE_LOG(LogTemp, Warning, TEXT("=== Find Session Complete ==="));
	UE_LOG(LogTemp, Warning, TEXT("Success: %s"), IsSuccess ? TEXT("true") : TEXT("false"));

	if (IsSuccess && SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Total Sessions Found: %d"), SessionSearch->SearchResults.Num());

		ServerNames.Empty();
		int32 SessionIndex = 0;

		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			if (!SearchResult.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("Session %d is invalid"), SessionIndex);
				SessionIndex++;
				continue;
			}

			// 빌드 ID 체크 주석 처리 또는 제거
			/*
			// 빌드 버전 체크
			if (SearchResult.Session.SessionSettings.BuildUniqueId != GetBuildUniqueId())
			{
				UE_LOG(LogTemp, Warning, TEXT("Session %d: Build mismatch - Server: 0x%08x, Client: 0x%08x"),
					SessionIndex,
					SearchResult.Session.SessionSettings.BuildUniqueId,
					GetBuildUniqueId());
				SessionIndex++;
				continue;
			}
			*/

			//GameTag 체크
			FString Temp_GameTag;
			bool bFoundTag = SearchResult.Session.SessionSettings.Get(FName("GAME_TAG_KEY"), Temp_GameTag);

			UE_LOG(LogTemp, Warning, TEXT("Session %d: GameTag = %s"), SessionIndex, *Temp_GameTag);

			if (!bFoundTag || (!Temp_GameTag.IsEmpty() && Temp_GameTag != GameUniqueTag))
			{
				UE_LOG(LogTemp, Warning, TEXT("Session %d: GameTag mismatch or empty"), SessionIndex);
				SessionIndex++;
				continue;
			}

			// 나머지 로직
			FString Temp_SessionName;
			SearchResult.Session.SessionSettings.Get(FName("ROOM_NAME_KEY"), Temp_SessionName);

			if (!SearchName.IsEmpty() && SearchName != Temp_SessionName)
			{
				SessionIndex++;
				continue;
			}

			bool Temp_bIsPublic = true;
			SearchResult.Session.SessionSettings.Get(FName("IS_PUBLIC_KEY"), Temp_bIsPublic);

			FString Temp_Password;
			SearchResult.Session.SessionSettings.Get(FName("PASSWORD_KEY"), Temp_Password);

			FServerData ServerData;
			ServerData.Accessibility = Temp_bIsPublic;
			ServerData.Password = Temp_Password;
			ServerData.HostUserName = SearchResult.Session.OwningUserName;
			ServerData.Name = Temp_SessionName;
			ServerData.CurrentPlayers = SearchResult.Session.SessionSettings.NumPublicConnections
				- SearchResult.Session.NumOpenPublicConnections;
			ServerData.SearchResultIndex = SessionIndex;

			FString ServerName;
			if (SearchResult.Session.SessionSettings.Get(FName("SERVER_NAME_KEY"), ServerName))
			{
				ServerData.Name = ServerName;
			}

			ServerNames.Add(ServerData);
			UE_LOG(LogTemp, Warning, TEXT("? Session added: %s"), *ServerData.Name);

			SessionIndex++;
		}

		UE_LOG(LogTemp, Warning, TEXT("=== Final session count: %d ==="), ServerNames.Num());
		OnSessionListUpdated.Broadcast();
	}
}

void UMyGameInstance::OnJoinSessioncomplete(FName InSessionName, EOnJoinSessionCompleteResult::Type InResult)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Cyan,
			TEXT("=========== OnJoinSessionComplete ==========="), true, FVector2D(2.0f, 2.0f));
	}

	// 결과 타입 확인
	FString ResultString;
	FColor ResultColor;

	switch (InResult)
	{
	case EOnJoinSessionCompleteResult::Success:
		ResultString = TEXT("SUCCESS");
		ResultColor = FColor::Green;
		break;
	case EOnJoinSessionCompleteResult::SessionIsFull:
		ResultString = TEXT("Session is Full");
		ResultColor = FColor::Red;
		break;
	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		ResultString = TEXT("Session Does Not Exist");
		ResultColor = FColor::Red;
		break;
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		ResultString = TEXT("Could Not Retrieve Address");
		ResultColor = FColor::Red;
		break;
	case EOnJoinSessionCompleteResult::AlreadyInSession:
		ResultString = TEXT("Already In Session");
		ResultColor = FColor::Red;
		break;
	default:
		ResultString = TEXT("Unknown Error");
		ResultColor = FColor::Red;
		break;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, ResultColor,
			FString::Printf(TEXT("Join Result: %s"), *ResultString),
			true, FVector2D(2.0f, 2.0f));
	}

	if (InResult != EOnJoinSessionCompleteResult::Success)
	{
		return;
	}

	if (!SessionInterface.IsValid())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red,
				TEXT("SessionInterface is NOT VALID!"), true, FVector2D(2.0f, 2.0f));
		}
		return;
	}

	FString Address;
	if (!SessionInterface->GetResolvedConnectString(InSessionName, Address))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red,
				TEXT("Could not get connect string!"), true, FVector2D(2.0f, 2.0f));
		}
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Yellow,
			FString::Printf(TEXT("Connect Address: %s"), *Address),
			true, FVector2D(1.5f, 1.5f));
	}

	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC == nullptr)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red,
				TEXT("PlayerController is NULL!"), true, FVector2D(2.0f, 2.0f));
		}
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Magenta,
			TEXT(">>> Calling ClientTravel() <<<"), true, FVector2D(2.0f, 2.0f));
	}

	PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void UMyGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
}