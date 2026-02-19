// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "OnlineSessionSettings.h"
#include <Kismet/GameplayStatics.h>
#include "SocketSubsystem.h"

//초보채널,중수채널
const static FName SESSION_NAME = TEXT("GameSession"); //채널명
const static FName SESSION_SETTINGS_KEY = TEXT("FREE");//게임모드

// Steam 호환 세션 키 정의
static const FName SETTING_SERVER_NAME = FName(TEXT("SERVER_NAME_KEY"));
static const FName SETTING_GAME_TAG = FName(TEXT("GAME_TAG_KEY"));
static const FName SETTING_IS_PUBLIC = FName(TEXT("IS_PUBLIC_KEY"));
static const FName SETTING_ROOM_NAME = FName(TEXT("ROOM_NAME_KEY"));
static const FName SETTING_PASSWORD = FName(TEXT("PASSWORD_KEY"));
static const FName SETTING_SERVER_PORT = FName(TEXT("SERVER_PORT")); // 포트 동기화용
static const FName SETTING_MAP_PATH = FName(TEXT("MAP_PATH_KEY")); //맵 동기화용

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

		// NULL 서브시스템 체크 (로컬 테스트용)
		if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
		{
			SessionSettings.bIsLANMatch = true;
			UE_LOG(LogTemp, Warning, TEXT("Creating LAN Session (NULL subsystem)"));
		}
		else
		{
			SessionSettings.bIsLANMatch = false;
			UE_LOG(LogTemp, Warning, TEXT("Creating Online Session"));
		}

		SessionSettings.NumPublicConnections = 4;
		SessionSettings.bUsesPresence = true;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bAllowJoinInProgress = true;
		SessionSettings.bAllowJoinViaPresence = true;

		// 포트 정보 명시
		SessionSettings.Set(SETTING_SERVER_PORT, 7777,
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		// Steam 호환 키를 사용한 세션 설정
		SessionSettings.Set(
			SETTING_SERVER_NAME, DesiredServerName,
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		//게임 식별 태그
		SessionSettings.Set(
			SETTING_GAME_TAG, GameUniqueTag,
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		//게임 접근 태그
		SessionSettings.Set(
			SETTING_IS_PUBLIC, bIsPublic,
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		//세션 이름과 비밀번호
		SessionSettings.Set(
			SETTING_ROOM_NAME, RoomName,
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		SessionSettings.Set(
			SETTING_PASSWORD, Password,
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		//맵
		SessionSettings.Set(
			SETTING_MAP_PATH, FString(TEXT("/Game/Main/PS_CJH/BuildObjects/Main")),
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		UE_LOG(LogTemp, Warning, TEXT("Creating session: %s on port 7777"), *DesiredServerName);

		//방생성
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
		SessionSearch->TimeoutInSeconds = 10.0f;

		// NULL 서브시스템 체크
		if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
		{
			SessionSearch->bIsLanQuery = true;
			UE_LOG(LogTemp, Warning, TEXT("Searching LAN sessions"));
		}
		else
		{
			SessionSearch->bIsLanQuery = false;
			UE_LOG(LogTemp, Warning, TEXT("Searching Online sessions"));
		}

		SessionSearch->QuerySettings.Set(FName(TEXT("PRESENCESEARCH")), true, EOnlineComparisonOp::Equals);
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UMyGameInstance::Join(int32 Index)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("SessionInterface is invalid!"));
		return;
	}

	if (!SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("SessionSearch is invalid!"));
		return;
	}

	// 기존 세션이 있으면 먼저 제거
	auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
	if (ExistingSession)
	{
		UE_LOG(LogTemp, Warning, TEXT("Destroying existing client session before join"));
		PendingJoinIndex = Index; // 인덱스 저장
		SessionInterface->DestroySession(SESSION_NAME);
		return; // 여기서 리턴! Destroy 완료 후 콜백에서 처리
	}

	// 기존 세션 없을 때는 바로 Join
	if (SessionSearch->SearchResults.Num() > Index)
	{
		SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
	}
}

void UMyGameInstance::OnCreateSessioncomplete(FName InSessionName, bool IsSuccess)
{
	if (!IsSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not Create session"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Session created successfully: %s"), *InSessionName.ToString());

	UWorld* World = GetWorld();
	if (!World) return;

	//레벨(맵) - ?listen 옵션으로 서버 모드 활성화
	World->ServerTravel("/Game/Main/PS_CJH/BuildObjects/Main?listen");
}


void UMyGameInstance::StartSession()
{
	if (SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Starting session..."));
		SessionInterface->StartSession(SESSION_NAME);
	}
}

void UMyGameInstance::OnDestroySessioncomplete(FName InSessionName, bool IsSuccess)
{
	if (!IsSuccess) return;

	if (bRecreateAfterDestroy)
	{
		bRecreateAfterDestroy = false;
		CreateSession();
	}
	else if (PendingJoinIndex >= 0)
	{
		int32 IndexToJoin = PendingJoinIndex;
		PendingJoinIndex = -1;
		Join(IndexToJoin); // 안전하게 재호출
	}
}

void UMyGameInstance::OnFindSessioncomplete(bool IsSuccess)
{
	UE_LOG(LogTemp, Warning, TEXT("=== OnFindSessionComplete ==="));
	UE_LOG(LogTemp, Warning, TEXT("Success: %d"), IsSuccess ? 1 : 0);

	if (IsSuccess && SessionSearch.IsValid())
	{
		int32 TotalResults = SessionSearch->SearchResults.Num();
		UE_LOG(LogTemp, Warning, TEXT("Total sessions found: %d"), TotalResults);

		ServerNames.Empty();
		int32 SessionIndex = 0;

		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("--- Processing Session %d ---"), SessionIndex);

			if (SearchResult.Session.SessionInfo.IsValid())
			{
				FString SessionIP = SearchResult.Session.SessionInfo->ToString();
				UE_LOG(LogTemp, Warning, TEXT("  Session IP: %s"), *SessionIP);
			}

			int32 ServerPort = 7777;
			SearchResult.Session.SessionSettings.Get(SETTING_SERVER_PORT, ServerPort);
			UE_LOG(LogTemp, Warning, TEXT("  Session Port: %d"), ServerPort);

			FString Temp_GameTag;
			SearchResult.Session.SessionSettings.Get(SETTING_GAME_TAG, Temp_GameTag);

			if (Temp_GameTag != GameUniqueTag || Temp_GameTag.IsEmpty())
			{
				UE_LOG(LogTemp, Warning, TEXT("  GameTag mismatch - Skipping"));
				SessionIndex++;
				continue;
			}

			FString Temp_SessionName;
			SearchResult.Session.SessionSettings.Get(SETTING_ROOM_NAME, Temp_SessionName);

			if (!SearchName.IsEmpty() && SearchName != Temp_SessionName)
			{
				UE_LOG(LogTemp, Warning, TEXT("  Room name filter mismatch - Skipping"));
				SessionIndex++;
				continue;
			}

			bool Temp_bIsPublic = true;
			SearchResult.Session.SessionSettings.Get(SETTING_IS_PUBLIC, Temp_bIsPublic);

			FString Temp_Password;
			SearchResult.Session.SessionSettings.Get(SETTING_PASSWORD, Temp_Password);

			FServerData ServerData;
			ServerData.Accessibility = Temp_bIsPublic;
			ServerData.Password = Temp_Password;
			ServerData.HostUserName = SearchResult.Session.OwningUserName;
			ServerData.Name = Temp_SessionName;
			ServerData.CurrentPlayers = SearchResult.Session.SessionSettings.NumPublicConnections
				- SearchResult.Session.NumOpenPublicConnections;
			ServerData.Port = ServerPort;

			FString ServerName;
			if (SearchResult.Session.SessionSettings.Get(SETTING_SERVER_NAME, ServerName))
			{
				ServerData.Name = ServerName;
				UE_LOG(LogTemp, Warning, TEXT("  Server Name: %s"), *ServerName);
			}

			ServerData.SearchResultIndex = SessionIndex;
			ServerNames.Add(ServerData);

			SessionIndex++;
		}

		UE_LOG(LogTemp, Warning, TEXT("=== Total valid sessions: %d ==="), ServerNames.Num());

		if (ServerNames.Num() > 0)
		{
			OnSessionListUpdated.Broadcast();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Search failed or no results!"));
	}
}

void UMyGameInstance::OnJoinSessioncomplete(FName InSessionName, EOnJoinSessionCompleteResult::Type InResult)
{
	UE_LOG(LogTemp, Warning, TEXT("=== OnJoinSessionComplete ==="));

	FString ResultString;
	switch (InResult)
	{
	case EOnJoinSessionCompleteResult::Success: ResultString = TEXT("SUCCESS"); break;
	case EOnJoinSessionCompleteResult::SessionIsFull: ResultString = TEXT("Session is Full"); break;
	case EOnJoinSessionCompleteResult::SessionDoesNotExist: ResultString = TEXT("Session Does Not Exist"); break;
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress: ResultString = TEXT("Could Not Retrieve Address"); break;
	case EOnJoinSessionCompleteResult::AlreadyInSession: ResultString = TEXT("Already In Session"); break;
	default: ResultString = TEXT("Unknown Error"); break;
	}

	UE_LOG(LogTemp, Warning, TEXT("Join Result: %s"), *ResultString);

	if (InResult != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to join session!"));
		return;
	}

	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("SessionInterface is invalid!"));
		return;
	}

	FString Address;
	if (!SessionInterface->GetResolvedConnectString(InSessionName, Address))
	{
		UE_LOG(LogTemp, Error, TEXT("Could not get connect string!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Original Address from GetResolvedConnectString: %s"), *Address);

	TArray<FString> AddressParts;
	Address.ParseIntoArray(AddressParts, TEXT(":"));

	if (AddressParts.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("No port in address! Adding default port 7777"));
		Address += TEXT(":7777");
	}
	else
	{
		int32 Port = FCString::Atoi(*AddressParts[1]);
		if (Port == 0 || Port < 1024 || Port > 65535)
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid port! Replacing with 7777"));
			Address = AddressParts[0] + TEXT(":7777");
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Final Connect Address: %s"), *Address);



	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerController is null!"));
		return;
	}

	// Address 확정 후, ClientTravel 직전에 추가
	FString MapPath;
	FName SessionName = InSessionName;

	// SearchResults에서 맵 경로 읽기
	if (SessionSearch.IsValid())
	{
		for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
		{
			FString Tag;
			Result.Session.SessionSettings.Get(SETTING_GAME_TAG, Tag);
			if (Tag == GameUniqueTag)
			{
				Result.Session.SessionSettings.Get(SETTING_MAP_PATH, MapPath);
				break;
			}
		}
	}

	if (!MapPath.IsEmpty())
	{
		Address = Address + MapPath; // "192.168.0.152:7777/Game/Main/PS_CJH/BuildObjects/Main"
	}

	UE_LOG(LogTemp, Warning, TEXT("Final Connect Address: %s"), *Address);
	PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void UMyGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Error, TEXT("=== Network Failure ==="));
	UE_LOG(LogTemp, Error, TEXT("Type: %d"), (int32)FailureType);
	UE_LOG(LogTemp, Error, TEXT("Error: %s"), *ErrorString);
}
