// Fill out your copyright notice in the Description page of Project Settings.

#include "KSM/MyGameInstance.h"
#include "OnlineSessionSettings.h"

const static FName SESSION_NAME = TEXT("GameSession"); //채널명



void UMyGameInstance::Init()
{
	Super::Init();
	GameUniqueTag = TEXT("SpaceMiningSimulator");

	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (OSS)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("OSS : %s is Avaliable."), *OSS->GetSubsystemName().ToString());

		SessionInterface = OSS->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnCreateSessionComplete);

			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnDestroySessionComplete);

			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnFindSessionComplete);

			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this,
				&UMyGameInstance::OnJoinSessionComplete);
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
	RoomName = ServerName;
	if (SessionInterface.IsValid())
	{
		auto AlreadyExsistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
		if (AlreadyExsistingSession)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s is already exsist. re-createSession."), *SESSION_NAME.ToString());
			SessionInterface->DestroySession(SESSION_NAME);
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
		//게임 식별 태그
		SessionSettings.Set(FName("GameUniqueTag"), GameUniqueTag, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		//게임 접근 태그
		SessionSettings.Set(TEXT("Public"), bIsPublic, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		//세션 이름과 비밀번호
		SessionSettings.Set(TEXT("SessionName"), RoomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		SessionSettings.Set(TEXT("Password"), Password, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
	}
}

void UMyGameInstance::PrintPublicConnectionNum()
{
	if (SessionInterface.IsValid())
	{
		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SESSION_NAME);
		if (Session)
		{
			UE_LOG(LogTemp, Error, TEXT("=== HOST Session Info ==="));
			UE_LOG(LogTemp, Error, TEXT("NumPublicConnections: %d"),
				Session->SessionSettings.NumPublicConnections);
			UE_LOG(LogTemp, Error, TEXT("NumOpenPublicConnections: %d"),
				Session->NumOpenPublicConnections);
			UE_LOG(LogTemp, Error, TEXT("NumPrivateConnections: %d"),
				Session->SessionSettings.NumPrivateConnections);
			UE_LOG(LogTemp, Error, TEXT("NumOpenPrivateConnections: %d"),
				Session->NumOpenPrivateConnections);
			UE_LOG(LogTemp, Error, TEXT("RegisteredPlayers: %d"),
				Session->RegisteredPlayers.Num());

			// 등록된 플레이어 목록 출력
			for (const FUniqueNetIdRef& PlayerId : Session->RegisteredPlayers)
			{
				UE_LOG(LogTemp, Error, TEXT("Registered Player: %s"),
					*PlayerId->ToString());
			}
		}

		UNetDriver* NetDriver = GetWorld()->GetNetDriver();
		if (NetDriver && NetDriver->IsServer())
		{
			UE_LOG(LogTemp, Log, TEXT("NetDriver is active and listening."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("NetDriver not initialized or not a server."));
		}
	}

	return;
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

void UMyGameInstance::Join(int Index)
{
	if (!SessionInterface.IsValid()) return;
	if (!SessionSearch.IsValid()) return;

	if (SessionSearch->SearchResults.Num() > (int32)Index)
		SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
	else
		UE_LOG(LogTemp, Warning, TEXT("Empty Session"));
}

void UMyGameInstance::OnCreateSessionComplete(FName InSessionName, bool IsSuccess)
{
	if (!IsSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not Createsession"));
		return;
	}

	UEngine* Engine = GetEngine();
	if (!Engine) return;

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

void UMyGameInstance::OnDestroySessionComplete(FName InSessionName, bool IsSuccess)
{
	if (IsSuccess == true)
		CreateSession();
}

void UMyGameInstance::OnFindSessionComplete(bool IsSuccess)
{
	if (IsSuccess && SessionSearch.IsValid())
	{
		ServerNames.Empty();
		int temp_index = 0;

		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Display, TEXT("Found Session name : %s"), *SearchResult.GetSessionIdStr());
			UE_LOG(LogTemp, Display, TEXT("Ping : %d"), SearchResult.PingInMs);

			FString tempName;
			SearchResult.Session.SessionSettings.Get(FName("SessionName"), tempName);
			if (!SearchName.IsEmpty() && SearchName != tempName)
				continue;

			FServerData ServerData;
			ServerData.CurrentPlayers = SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections;
			ServerData.HostUserName = SearchResult.Session.OwningUserName;
			SearchResult.Session.SessionSettings.Get(FName("Password"), ServerData.Password);
			SearchResult.Session.SessionSettings.Get(FName("SessionName"), ServerData.Name);
			SearchResult.Session.SessionSettings.Get(FName("Public"), ServerData.Accessibility);

			temp_index++;
			ServerNames.Add(ServerData);
		}

		UE_LOG(LogTemp, Warning, TEXT("Finished Finding Session"));
		OnSessionListUpdated.Broadcast();
	}
}

void UMyGameInstance::OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type InResult)
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

	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC == nullptr) return;
	PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void UMyGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
}