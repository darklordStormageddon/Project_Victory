// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ResourcePath.generated.h"

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
	{}

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

USTRUCT(BlueprintType)
struct FResourceUI
{
	GENERATED_BODY()

public:
	FResourceUI()
		: UI_BLUEPRINT_BASE_PATH(TEXT("/Game/Main/PS_JHS/Blueprint/"))
		, UI_PANEL_FOLDER(TEXT("UI/Panel/"))
		, UI_POPUP_FOLDER(TEXT("UI/Popup/Common/"))
		, UI_SYSTEM_FOLDER(TEXT("UI/System/"))
		, UI_WIDGET_HEADER(TEXT("WBP_"))
	{}

	// UI Blueprint
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constance|Resource|UI")
	FString UI_BLUEPRINT_BASE_PATH;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constance|Resource|UI")
	FString UI_PANEL_FOLDER;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constance|Resource|UI")
	FString UI_POPUP_FOLDER;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constance|Resource|UI")
	FString UI_SYSTEM_FOLDER;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constance|Resource|UI")
	FString UI_WIDGET_HEADER;
};

USTRUCT(BlueprintType)
struct FResourceImage
{
	GENERATED_BODY()

public:
	FResourceImage()
		: IMAGE_INTERACT_FOLDER_PATH(TEXT("/Game/Main/PS_JHS/Resource/Images/Interact/"))
	{
	}

	// UI Blueprint
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constance|Resource|Image")
	FString IMAGE_INTERACT_FOLDER_PATH;
};

/**
 * 
 */
class ResourcePath
{
public:
	ResourcePath();
	~ResourcePath();

	FResourceSpaceObject SpaceObject;
	FResourceUI UI;
	FResourceImage Image;
};
