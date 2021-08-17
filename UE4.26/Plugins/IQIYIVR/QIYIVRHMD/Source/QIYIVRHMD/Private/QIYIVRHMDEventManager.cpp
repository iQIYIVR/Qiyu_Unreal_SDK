//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================
#include "../Classes/QIYIVRHMDEventManager.h"

static UQIYIVRHMDEventManager* Singleton = nullptr;

UQIYIVRHMDEventManager::UQIYIVRHMDEventManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UQIYIVRHMDEventManager* UQIYIVRHMDEventManager::GetInstance()
{
	if (!Singleton)
	{
		Singleton = NewObject<UQIYIVRHMDEventManager>();
		Singleton->AddToRoot();
	}
	return Singleton;
}