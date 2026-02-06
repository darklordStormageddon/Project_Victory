// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/CommonEnums.h"

CommonEnums::CommonEnums()
{
}

CommonEnums::~CommonEnums()
{
}

bool CommonEnums::TryGetInteractType(FString InEnumName, E_INTERACT_TYPE& OutInteractType)
{
    const FString _noneName = GetEnum2FString<E_INTERACT_TYPE>(E_INTERACT_TYPE::NONE);
    if (InEnumName == _noneName)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid ammo type [None]"));
        return false;
    }

    const int32 _maxIndex = static_cast<int32>(E_INTERACT_TYPE::NONE);
    for (int32 i = 0; i < _maxIndex; i++)
    {
        E_INTERACT_TYPE _enum = (E_INTERACT_TYPE)i;
        if (InEnumName == GetEnum2FString<E_INTERACT_TYPE>(_enum))
        {
            OutInteractType = _enum;
            return true;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Invalid ammo type [%s]"), *InEnumName);
    return false;
}

bool CommonEnums::TryGetElementType(FString InEnumName, E_ELEMENT_TYPE& OutElementType)
{
    const FString _noneName = GetEnum2FString<E_ELEMENT_TYPE>(E_ELEMENT_TYPE::NONE);
    if (InEnumName == _noneName)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid element type [None]"));
        return false;
    }

    const int32 _maxIndex = static_cast<int32>(E_ELEMENT_TYPE::NONE);
    for (int32 i = 0; i < _maxIndex; i++)
    {
        E_ELEMENT_TYPE _enum = (E_ELEMENT_TYPE)i;
        if (InEnumName == GetEnum2FString<E_ELEMENT_TYPE>(_enum))
        {
            OutElementType = _enum;
            return true;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Invalid element type [%s]"), *InEnumName);
    return false;
}