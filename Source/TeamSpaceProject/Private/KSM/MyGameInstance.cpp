// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "OnlineSessionSettings.h"
#include <Kismet/GameplayStatics.h>

//초보채널,중수채널
const static FName SESSION_NAME = TEXT("GameSession"); //채널명
const static FName SESSION_SETTINGS_KEY = TEXT("FREE");//게임모드

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
		SessionSettings.Set(
			SESSION_SETTINGS_KEY, DesiredServerName,
			EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		//게임 식별 태그
		SessionSettings.Set(FName("GameUniqueTag"), GameUniqueTag, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		//게임 접근 태그
		SessionSettings.Set(TEXT("Public"), bIsPublic, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		//세션 이름과 비밀번호
		SessionSettings.Set(TEXT("SessionName"), RoomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		SessionSettings.Set(TEXT("Password"), Password, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);


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
	//World->ServerTravel("/Game/Import/Maps/Lobby?listen");
	World->ServerTravel("/Game/Import/Maps/Lobby?listen?game=/Game/Main/PS_KSM/GameSettings/BP_TempGameMode_C");
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

			FString Temp_GameTag;
			SearchResult.Session.SessionSettings.Get(FName("GameUniqueTag"), Temp_GameTag);

			if (Temp_GameTag != GameUniqueTag || Temp_GameTag.IsEmpty())
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Different Game!");
				continue;
			}

			FString Temp_SessionName;
			SearchResult.Session.SessionSettings.Get(FName("SessionName"), Temp_SessionName);
			if (!SearchName.IsEmpty() && SearchName != Temp_SessionName)
				continue;

			bool Temp_bIsPublic;
			SearchResult.Session.SessionSettings.Get(FName("Public"), Temp_bIsPublic);

			FString Temp_Password;
			SearchResult.Session.SessionSettings.Get(FName("Password"), Temp_Password);



			FServerData ServerData;
			ServerData.Accessibility = Temp_bIsPublic;
			ServerData.Password = Temp_Password;
			ServerData.HostUserName = SearchResult.Session.OwningUserName;
			ServerData.Name = NickName;

			ServerData.CurrentPlayers = SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections;

			FString ServerName;
			if (SearchResult.Session.SessionSettings.Get(SESSION_SETTINGS_KEY, ServerName))
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