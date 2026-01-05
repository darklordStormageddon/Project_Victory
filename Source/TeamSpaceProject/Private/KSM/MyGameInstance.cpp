// Fill out your copyright notice in the Description page of Project Settings.


#include "KSM/MyGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "MyGameInstanceSubsystem.h"

void UMyGameInstance::Init()
{
    Super::Init();
    // 여기서 Subsystem 초기화 가능
	GIS = GetSubsystem<UMyGameInstanceSubsystem>();
}

void UMyGameInstance::HostServer()
{
    if (GIS)
    {
        GIS->HostServer(IsPublic);
    }
}

void UMyGameInstance::FindServers()
{
    GIS->FindSessions();
}

void UMyGameInstance::RefreshServers()
{
    GIS->RefreshSessions();
}

void UMyGameInstance::JoinServer(FString ServerAddress)
{
    UWorld* World = GetWorld();
	GIS->JoinServer(ServerAddress);
    if (World)
    {
        APlayerController* PC = GetFirstLocalPlayerController();
        if (PC)
        {
            PC->ClientTravel(ServerAddress, ETravelType::TRAVEL_Absolute);
        }
    }
}