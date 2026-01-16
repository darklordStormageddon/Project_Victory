// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/CommonEnums.h"

CommonEnums::CommonEnums()
{
}

CommonEnums::~CommonEnums()
{
}

bool CommonEnums::TryGetAmmoType(FString InEnumName, E_AMMO_TYPE& OutAmmoType)
{
	const FString _noneName = GetEnum2FString<E_AMMO_TYPE>(E_AMMO_TYPE::NONE);
	if (InEnumName == _noneName)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid ammo type [None]"));
        return false;
    }

	const int32 _maxIndex = static_cast<int32>(E_AMMO_TYPE::NONE);
    for (int32 i = 0; i < _maxIndex; i++)
    {
        E_AMMO_TYPE _enum = (E_AMMO_TYPE)i;
        if (InEnumName == GetEnum2FString<E_AMMO_TYPE>(_enum))
        {
            OutAmmoType = _enum;
            return true;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Invalid ammo type [%s]"), *InEnumName);
    return false;
}