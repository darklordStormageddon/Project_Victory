// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ResourcePath.generated.h"

USTRUCT(BlueprintType)
struct FResourceDataTable
{
	GENERATED_BODY()

public:
	FResourceDataTable()
		: TURRET_INFO_PATH(TEXT("/Game/Main/PS_YSH/Data/DT_TurretDataTable.DT_TurretDataTable"))
		, ELEMENT_INFO_PATH(TEXT("/Game/Main/PS_KSM/"))
	{}

	UPROPERTY()
	FString TURRET_INFO_PATH;

	UPROPERTY()
	FString ELEMENT_INFO_PATH;
};

USTRUCT(BlueprintType)
struct FResourceUI
{
	GENERATED_BODY()

public:
	FResourceUI()
		: BLUEPRINT_BASE_PATH(TEXT("/Game/Main/PS_JHS/Blueprint/"))
		, UI_PANEL_FOLDER(TEXT("UI/Panel/"))
		, UI_POPUP_FOLDER(TEXT("UI/Popup/Common/"))
		, UI_SYSTEM_FOLDER(TEXT("UI/System/"))
		, UI_WIDGET_HEADER(TEXT("WBP_"))
	{}

	// UI Blueprint
	UPROPERTY()
	FString BLUEPRINT_BASE_PATH;

	UPROPERTY()
	FString UI_PANEL_FOLDER;

	UPROPERTY()
	FString UI_POPUP_FOLDER;

	UPROPERTY()
	FString UI_SYSTEM_FOLDER;

	UPROPERTY()
	FString UI_WIDGET_HEADER;
};

USTRUCT(BlueprintType)
struct FResourceImage
{
	GENERATED_BODY()

public:
	FResourceImage()
		: TEXTURE_HEADER(TEXT("Tex_"))
		, INTERACT_FOLDER_PATH(TEXT("/Game/Main/PS_JHS/Resource/Images/Interact/"))
		, ELEMENT_FOLDER_PATH(TEXT("/Game/Main/PS_KSM/"))
		, AMMO_FOLDER_PATH(TEXT("/Game/Main/PS_JHS/Resource/Images/AmmoType/"))
	{}

	UPROPERTY()
	FString TEXTURE_HEADER;;

	UPROPERTY()
	FString INTERACT_FOLDER_PATH;

	UPROPERTY()
	FString ELEMENT_FOLDER_PATH;

	UPROPERTY()
	FString AMMO_FOLDER_PATH;
};

USTRUCT(BlueprintType)
struct FResourceSpaceObject
{
	GENERATED_BODY()

public:
	FResourceSpaceObject()
		: RADER_MESH_FOLDER_PATH(TEXT("/Game/Main/PS_JHS/Resource/RaderMesh/"))
		, SPACE_RADER_FOLDER(TEXT("SpaceRader/"))
		, SPACE_RADER_HEADER(TEXT("BP_SR"))
		, DRIVE_RADER_FOLDER(TEXT("DriveRader/"))
		, DRIVE_RADER_HEADER(TEXT("BP_DR"))
	{
	}

	// Rader Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constance|Resource|SpaceObject")
	FString RADER_MESH_FOLDER_PATH;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constance|Resource|SpaceObject")
	FString SPACE_RADER_FOLDER;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constance|Resource|SpaceObject")
	FString SPACE_RADER_HEADER;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constance|Resource|SpaceObject")
	FString DRIVE_RADER_FOLDER;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constance|Resource|SpaceObject")
	FString DRIVE_RADER_HEADER;
};

class ResourcePath
{
public:
	ResourcePath();
	~ResourcePath();

	FResourceDataTable DataTable;
	FResourceUI UI;
	FResourceImage Image;
	FResourceSpaceObject SpaceObject;
};
