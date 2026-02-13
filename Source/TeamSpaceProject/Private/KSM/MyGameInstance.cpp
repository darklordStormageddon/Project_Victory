// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "OnlineSessionSettings.h"
#include <Kismet/GameplayStatics.h>

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
		SessionSettings.bUsesPresence = SessionSettings.bShouldAdvertise = true;
		SessionSettings.bAllowJoinInProgress = true;
		SessionSettings.bAllowJoinViaPresence = true;

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
		SessionSearch->QuerySettings.Set(FName(TEXT("PRESENCESEARCH")), true, EOnlineComparisonOp::Equals);
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

void UMyGameInstance::OnCreateSessioncomplete(FName InSessionName, bool IsSuccess)
{
	if (!IsSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not Createsession"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Session name is %s"), *InSessionName.ToString());

	UEngine* Engine = GetEngine();
	if (!Engine) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Host complete!"));

	UWorld* World = GetWorld();
	if (!World) return;

	//레벨(맵)
	World->ServerTravel("/Game/Import/Maps/Lobby?listen");
}


void UMyGameInstance::StartSession()
{
	if (SessionInterface.IsValid())
		SessionInterface->StartSession(SESSION_NAME);
}

void UMyGameInstance::OnDestroySessioncomplete(FName InSessionName, bool IsSuccess)
{
	if (IsSuccess == true && bRecreateAfterDestroy == false)
		CreateSession();
}

void UMyGameInstance::OnFindSessioncomplete(bool IsSuccess)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Search Complete!");
	if (IsSuccess && SessionSearch.IsValid())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, *FString::Printf(TEXT("Session Found : %d"), SessionSearch->SearchResults.Num()));
		ServerNames.Empty();
		int32 SessionIndex = 0;

		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, *FString::Printf(TEXT("Session Index : %d"), SessionIndex));

			// Steam 호환 키를 사용한 GameTag 파싱
			FString Temp_GameTag;
			SearchResult.Session.SessionSettings.Get(SETTING_GAME_TAG, Temp_GameTag);

			if (Temp_GameTag != GameUniqueTag || Temp_GameTag.IsEmpty())
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Different Game!");
				continue;
			}

			// Steam 호환 키를 사용한 SessionName 파싱
			FString Temp_SessionName;
			SearchResult.Session.SessionSettings.Get(SETTING_ROOM_NAME, Temp_SessionName);
			if (!SearchName.IsEmpty() && SearchName != Temp_SessionName)
				continue;

			// Steam 호환 키를 사용한 Public 여부 파싱
			bool Temp_bIsPublic;
			SearchResult.Session.SessionSettings.Get(SETTING_IS_PUBLIC, Temp_bIsPublic);

			// Steam 호환 키를 사용한 Password 파싱
			FString Temp_Password;
			SearchResult.Session.SessionSettings.Get(SETTING_PASSWORD, Temp_Password);

			FServerData ServerData;
			ServerData.Accessibility = Temp_bIsPublic;
			ServerData.Password = Temp_Password;
			ServerData.HostUserName = SearchResult.Session.OwningUserName;
			ServerData.Name = NickName;

			ServerData.CurrentPlayers = SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections;

			// Steam 호환 키를 사용한 ServerName 파싱
			FString ServerName;
			if (SearchResult.Session.SessionSettings.Get(SETTING_SERVER_NAME, ServerName))
				ServerData.Name = ServerName;
			else
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "SessionName not found!");

			ServerData.SearchResultIndex = SessionIndex;
			++SessionIndex;

			ServerNames.Add(ServerData);
			OnSessionListUpdated.Broadcast();
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Session Added to list!");
		}

	}
	else GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Search invalid!");
}

void UMyGameInstance::OnJoinSessioncomplete(FName InSessionName, EOnJoinSessionCompleteResult::Type InResult)
{
	if (SessionInterface.IsValid() == false) return;

	FString Address;//해당 방의 아이피주소
	if (!SessionInterface->GetResolvedConnectString(InSessionName, Address))
	{
		UE_LOG(LogTemp, Error, TEXT("Could not convert IP Address"));
		return;
	}
	UEngine* Engine = GetEngine();
	if (!Engine) return;
	Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining To %s"), *Address));

	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC == nullptr) return;
	PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void UMyGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
}