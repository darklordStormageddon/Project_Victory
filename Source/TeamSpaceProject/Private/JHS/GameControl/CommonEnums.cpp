// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/CommonEnums.h"

CommonEnums::CommonEnums()
{
}

CommonEnums::~CommonEnums()
{
}

FString CommonEnums::GetFStringInteractEnum(E_INTERACT_TYPE InteractType)
{
	switch (InteractType)
	{
	case E_INTERACT_TYPE::None:
		return "None";

	case E_INTERACT_TYPE::Chair:
		return "Chair";

	default:
		return "Invalid E_INTERACT_TYPE";
	}
}