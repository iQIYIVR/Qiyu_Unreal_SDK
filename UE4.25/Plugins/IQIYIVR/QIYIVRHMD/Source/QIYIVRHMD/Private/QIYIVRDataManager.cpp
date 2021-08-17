//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================

#include "QIYIVRDataManager.h"
#include "QiyuSettings.h"
#include "Json.h"



UQIYIVRDataManager* UQIYIVRDataManager::s_QIYIVRDataManager = nullptr;

UQIYIVRDataManager* UQIYIVRDataManager::GetInstance()
{
	if (nullptr == s_QIYIVRDataManager)
	{
		s_QIYIVRDataManager = NewObject<UQIYIVRDataManager>();
		s_QIYIVRDataManager->AddToRoot();
	}
	return s_QIYIVRDataManager;
}

UQIYIVRDataManager::UQIYIVRDataManager()
{
	indexRequestID = 0;
	RegisterListener();
}

UQIYIVRDataManager::~UQIYIVRDataManager()
{
}

void UQIYIVRDataManager::Request_InitQiyuSDK()
{
#if QIYIVR_SUPPORTED_PLATFORMS
	QiyiVRCore::QVR_InitQiyuSDK(TCHAR_TO_ANSI( *GetMutableDefault<UQiyuSettings>()->appID ),
		TCHAR_TO_ANSI( *GetMutableDefault<UQiyuSettings>()->developer_id ),
		TCHAR_TO_ANSI( *GetMutableDefault<UQiyuSettings>()->app_secret ),
		TCHAR_TO_ANSI( *GetMutableDefault<UQiyuSettings>()->sign_key) );
#endif
}

void UQIYIVRDataManager::Request_AccountInfo()
{
	AddRequest(
		FRequestCallbackByJson<FQIYIAccountInfo>::Create([](FString jsonStr) {


		TSharedRef< TJsonReader<TCHAR> > Reader = TJsonReaderFactory<TCHAR>::Create(jsonStr);
		TSharedPtr<FJsonObject> rRoot;
		if (FJsonSerializer::Deserialize(Reader, rRoot))
		{
			if (rRoot->HasField(TEXT("code")))
			{
				UQIYIVRDataManager::GetInstance()->GetQIYIAccountInfo().code = rRoot->GetStringField(TEXT("code"));
			}


			UQIYIVRDataManager::GetInstance()->GetQIYIAccountInfo().Init();
			if (!UQIYIVRDataManager::GetInstance()->GetQIYIAccountInfo().IsSuccess()) { return; }

			TSharedPtr<FJsonObject> dataObject = rRoot->GetObjectField("data");
			if (dataObject->HasField(TEXT("icon")))
			{
				UQIYIVRDataManager::GetInstance()->GetQIYIAccountInfo().icon = dataObject->GetStringField(TEXT("icon"));
			}
			if (dataObject->HasField(TEXT("name")))
			{
				UQIYIVRDataManager::GetInstance()->GetQIYIAccountInfo().name = dataObject->GetStringField(TEXT("name"));
			}
			if (dataObject->HasField(TEXT("uid")))
			{
				UQIYIVRDataManager::GetInstance()->GetQIYIAccountInfo().uid = dataObject->GetStringField(TEXT("uid"));
			}

		}
	})
		
	);
#if QIYIVR_SUPPORTED_PLATFORMS	

 	QiyiVRCore::QVR_GetQiyuAccountInfo(indexRequestID);
 	UE_LOG(LogTemp, Warning, TEXT("622Message: Get2"));
#endif

}

void UQIYIVRDataManager::Request_isAccountLogin()
{

#if QIYIVR_SUPPORTED_PLATFORMS
	b_isAccountLogin = (int32)QiyiVRCore::QVR_IsAccountLogin();
#endif

}

void UQIYIVRDataManager::Request_AppUpdateInfo(FString app_id, FString curVersion)
{
	AddRequest(
		FRequestCallbackByJson<FQIYIMessageResult<FQIYIAppUpdateInfo>>::Create([](FString jsonStr) {

		TSharedRef< TJsonReader<TCHAR> > Reader = TJsonReaderFactory<TCHAR>::Create(jsonStr);
		TSharedPtr<FJsonObject> rRoot;
		if (FJsonSerializer::Deserialize(Reader, rRoot))
		{

			if (rRoot->HasField(TEXT("code")))
			{
				UQIYIVRDataManager::GetInstance()->GetAppUpdateInfo().code = rRoot->GetStringField(TEXT("code"));
			}

			TSharedPtr<FJsonObject> dataObject = rRoot->GetObjectField("data");
			TArray<TSharedPtr<FJsonValue>> filesArray = dataObject->GetArrayField("files");

			for (size_t i = 0; i < filesArray.Num(); i++)
			{
				FQIYIAppUpdateInfo_Item s_AppUpdateInfo_Item;

				if (filesArray[i]->AsObject()->HasField(TEXT("originalName")))
				{
					s_AppUpdateInfo_Item.originalName = filesArray[i]->AsObject()->GetStringField(TEXT("originalName"));
				}
				if (filesArray[i]->AsObject()->HasField(TEXT("downUrl")))
				{
					s_AppUpdateInfo_Item.downUrl = filesArray[i]->AsObject()->GetStringField(TEXT("downUrl"));
				}
				if (filesArray[i]->AsObject()->HasField(TEXT("updateTime")))
				{
					s_AppUpdateInfo_Item.updateTime = filesArray[i]->AsObject()->GetStringField(TEXT("updateTime"));
				}
				UQIYIVRDataManager::GetInstance()->GetAppUpdateInfo().files.Add(s_AppUpdateInfo_Item);
			}

		}

	})
	);
#if QIYIVR_SUPPORTED_PLATFORMS
	QiyiVRCore::QVR_GetAppUpdateInfo(indexRequestID,
		TCHAR_TO_ANSI(*app_id),
		TCHAR_TO_ANSI(*curVersion));
#endif
}


void UQIYIVRDataManager::Request_LaunchOtherApp(FString app_id, FString key, FString value)
{
#if QIYIVR_SUPPORTED_PLATFORMS
	QiyiVRCore::QVR_LaunchOtherApp(TCHAR_TO_ANSI(*app_id), 
								   TCHAR_TO_ANSI(*key),		
								   TCHAR_TO_ANSI(*value));
#endif
}

void UQIYIVRDataManager::Request_DeepLinkParam()
{
	AddRequest(
	FRequestCallbackByJson<FQIYIMessageResult<FQIYIDeepLinkParam>>::Create([](FString jsonStr) {
		
		TSharedRef< TJsonReader<TCHAR> > Reader = TJsonReaderFactory<TCHAR>::Create(jsonStr);
		TSharedPtr<FJsonObject> rRoot;
		if (FJsonSerializer::Deserialize(Reader, rRoot))
		{
			if (rRoot->HasField(TEXT("code")))
			{
				UQIYIVRDataManager::GetInstance()->GetDeepLinkParam().code = rRoot->GetStringField(TEXT("code"));
			}

			if (!UQIYIVRDataManager::GetInstance()->GetQIYIAccountInfo().IsSuccess()) { return; }

			TSharedPtr<FJsonObject> dataObject = rRoot->GetObjectField("data");
			if (dataObject->HasField(TEXT("appID")))
			{
				UQIYIVRDataManager::GetInstance()->GetDeepLinkParam().appID = dataObject->GetStringField(TEXT("appID"));
			}
			if (dataObject->HasField(TEXT("key")))
			{
				UQIYIVRDataManager::GetInstance()->GetDeepLinkParam().key = dataObject->GetStringField(TEXT("key"));
			}
			if (dataObject->HasField(TEXT("value")))
			{
				UQIYIVRDataManager::GetInstance()->GetDeepLinkParam().value = dataObject->GetStringField(TEXT("value"));
			}
		}
	})
	);
#if QIYIVR_SUPPORTED_PLATFORMS
	QiyiVRCore::QVR_GetDeepLink(indexRequestID);
#endif
}


void UQIYIVRDataManager::Request_LaunchHome()
{
#if QIYIVR_SUPPORTED_PLATFORMS
	QiyiVRCore::QVR_LaunchHome("login","	");
#endif
}


void UQIYIVRDataManager::MessageProcess(uint64 messageID, const char* result, int32 resultSize)
{ 
	if (s_QIYIVRDataManager->m_requestListMap.Contains(messageID))
	{
		s_QIYIVRDataManager->m_requestListMap[messageID]->HandleRequestMessage(result, resultSize);

		s_QIYIVRDataManager->m_requestListMap.Remove(messageID);
		
	}
}

void UQIYIVRDataManager::ListenerProcess(uint64 messageID, const char* result, int32 resultSize)
{
	//test string 
	s_QIYIVRDataManager->temp = FString(result);
	if (s_QIYIVRDataManager->m_listenerListMap.Contains((int32)messageID))
	{
		s_QIYIVRDataManager->m_listenerListMap[messageID]->HandleRequestMessage(TCHAR_TO_UTF8(result), resultSize);
	}
}

void  UQIYIVRDataManager::RegisterListener()
{
	Request_InitQiyuSDK();
	AddListener(EQIYIMessageCode::QiyuSdkInit,
		FRequestCallbackByJson<FQIYIMessageResult<FQIYISDKInit>>::Create( [] (FString str) {

		TSharedRef< TJsonReader<TCHAR> > Reader = TJsonReaderFactory<TCHAR>::Create(str);
		TSharedPtr<FJsonObject> rRoot;
		if (FJsonSerializer::Deserialize(Reader, rRoot))
		{
			if (rRoot->HasField(TEXT("code")))
			{
				UQIYIVRDataManager::GetInstance()->GetQIYIAccountInfo().code = rRoot->GetStringField(TEXT("code"));
			}
			if (rRoot->HasField(TEXT("Message")))
			{
				UQIYIVRDataManager::GetInstance()->GetQIYIAccountInfo().Message = rRoot->GetStringField(TEXT("Message"));
			}
		}
		})
	);
	
}

int32 UQIYIVRDataManager::AddRequest(FRequestCallback* rcb)
{
	uint64 requestID = GetIndexRequestID();

	m_requestListMap.Add(requestID, rcb);	
	return requestID;
}

void UQIYIVRDataManager::AddListener(EQIYIMessageCode messageCode, FRequestCallback* rcb)
{
	if (!m_listenerListMap.Contains((int32)messageCode))
	{
		m_listenerListMap.Add(messageCode, rcb);
	}
}
