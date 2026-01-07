// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ConstantLibrary.generated.h"

USTRUCT(BlueprintType)
struct FResourcePath
{
	GENERATED_BODY()

public:
	FResourcePath()
		: RADER_MESH_FOLDER_PATH(TEXT("/Game/Main/PS_JHS/Resource/RaderMesh/"))
		, SPACE_RADER_FOLDER(TEXT("SpaceRader/"))
		, SPACE_RADER_HEADER(TEXT("BP_SR"))
		, DRIVE_RADER_FOLDER(TEXT("DriveRader/"))
		, DRIVE_RADER_HEADER(TEXT("BP_DR"))
	{}

	// Rader Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constance|Resource|Rader")
	FString RADER_MESH_FOLDER_PATH;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constance|Resource|Rader")
	FString SPACE_RADER_FOLDER;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constance|Resource|Rader")
	FString SPACE_RADER_HEADER;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constance|Resource|Rader")
	FString DRIVE_RADER_FOLDER;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constance|Resource|Rader")
	FString DRIVE_RADER_HEADER;
};

class ConstantLibrary
{
public:
	static FResourcePath Resource;
};
