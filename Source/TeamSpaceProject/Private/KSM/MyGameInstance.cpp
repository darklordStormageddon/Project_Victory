// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include <Kismet/GameplayStatics.h>
#include "Engine/NetConnection.h"
#include "Net/Core/Connection/NetCloseResult.h"
#include <Online/OnlineSessionNames.h>

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
	UE_LOG(LogTemp, Warning, TEXT("OnStartSessionComplete: %s, Success=%d"), *SessionName.ToString(), bWasSuccessful ? 1 : 0);

	if (!bWasSuccessful)
		return;

	UWorld* World = GetWorld();
	if (!World) return;

	UE_LOG(LogTemp, Warning, TEXT("Performing ServerTravel to Lobby..."));
	World->ServerTravel(TEXT("/Game/Import/Maps/Lobby?listen"));
}

void UMyGameInstance::OnCreateSessioncomplete(FName InSessionName, bool IsSuccess)
{
	if (!IsSuccess) return;
	StartSession();
}

void UMyGameInstance::OnDestroySessioncomplete(FName InSessionName, bool IsSuccess)
{
	if (IsSuccess == true && bRecreateAfterDestroy == false)
		CreateSession();
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