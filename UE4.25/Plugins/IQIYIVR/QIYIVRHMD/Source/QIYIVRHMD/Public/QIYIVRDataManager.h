//=============================================================================
//
//      Copyright (c) 2021 Beijing iQIYI Intelligent Technologies Inc.
//                         All Rights Reserved.
//
//==============================================================================

#pragma once
#include "CoreMinimal.h"
#include "QIYIVRSDKCore.h"
#include "IQIYIVRHMDModule.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "QIYIVRDataManager.generated.h"

typedef unsigned long long uint64;
enum EQIYIMessageCode 
{
	QiyuSdkInit = 0,
	End
};

struct EQIYIMessageResultCode
{
	FString S0000 = "S0000";//正常
	FString S9000 = "S9000";//系统异常
	FString S8001 = "S8001";//账号未登陆 
	FString S8002 = "S8002";//未连接到Service
	FString S8003 = "S8003";//无效的appid
};

template <typename T>
struct FQIYIMessageResult
{
	T data;
	FString code;
	FString Message;
	bool IsSuccess()
	{
		return code == "S0000";
	}
};
struct FQIYISDKInit 
{
	
};

USTRUCT(BlueprintType)
struct FQIYIAccountInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiDataManager")
		FString code;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiDataManager")
		FString Message;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiDataManager")
		FString uid;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiDataManager")
		FString name;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiDataManager")
		FString icon;

	void Init()
	{
		uid  = "";
		name = "";
		icon = "";
	}

	bool IsSuccess()
	{
		return code == "S0000";
	}
};

USTRUCT(BlueprintType)
struct FQIYIAppUpdateInfo_Item
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiDataManager")
		FString originalName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiDataManager")
		FString downUrl;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiDataManager")
		FString updateTime;
};

USTRUCT(BlueprintType)
struct FQIYIAppUpdateInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiDataManager")
	FString code;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiDataManager")
	TArray<FQIYIAppUpdateInfo_Item> files;

	void Init()
	{
		files.Empty();
	}
	
	bool IsSuccess()
	{
		return code == "S0000";
	}
};

USTRUCT(BlueprintType)
struct FQIYIDeepLinkParam
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiDataManager")
		FString code;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiDataManager")
		FString appID;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiDataManager")
		FString key;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QiyiDataManager")
		FString value;

	void Init()
	{
		appID = "";
		key = "";
		value = "";
	}

	bool IsSuccess()
	{
		return code == "S0000";
	}

};

//callback class
class FRequestCallback
{
public:
	FRequestCallback() {};
	~FRequestCallback() {};
	virtual void HandleRequestMessage(FString data, int32 size) {

	};
};

template <typename T>
class FRequestCallbackByJson : public FRequestCallback
{
public:
	typedef void (*callBack)(FString);

public:
	FRequestCallbackByJson(callBack funcTemp) { f_callback = std::move(funcTemp); }
	~FRequestCallbackByJson() {};

	static FRequestCallback* Create(callBack funcTemp)
	{
		FRequestCallback* rcb = new FRequestCallbackByJson(funcTemp);

		return rcb;
	}
	virtual void HandleRequestMessage(FString data, int32 size)
	{
		f_callback(data);
	}
private:
	callBack f_callback;
};

UCLASS()
class UQIYIVRDataManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UQIYIVRDataManager();
	~UQIYIVRDataManager();
	UFUNCTION(BlueprintCallable, Category = "QiyiDataManager")
	static UQIYIVRDataManager* GetInstance();

	static UQIYIVRDataManager& GetInstanceRef() { return *s_QIYIVRDataManager; }

public:

	UFUNCTION(BlueprintCallable, Category = "QiyiDataManager")
		FQIYIAccountInfo& GetQIYIAccountInfo() { return s_QIYIAccountInfo; }

	UFUNCTION(BlueprintCallable, Category = "QiyiDataManager")
		FQIYIAppUpdateInfo& GetAppUpdateInfo() { return s_QIYIAppUpdateInfo; }

	UFUNCTION(BlueprintCallable, Category = "QiyiDataManager")
		FQIYIDeepLinkParam& GetDeepLinkParam() { return s_QIYIDeepLinkParam; }

	UFUNCTION(BlueprintCallable, Category = "QiyiDataManager")
		int32 GetIsAccountLogin() { return b_isAccountLogin; }

	UFUNCTION(BlueprintCallable, Category = "QiyiDataManager")
		void Request_AccountInfo(); 
	UFUNCTION(BlueprintCallable, Category = "QiyiDataManager")
		void  Request_isAccountLogin(); 
	UFUNCTION(BlueprintCallable, Category = "QiyiDataManager")
		void Request_AppUpdateInfo(FString app_id, FString curVersion);  
	UFUNCTION(BlueprintCallable, Category = "QiyiDataManager")
		void Request_LaunchOtherApp(FString app_id, FString key, FString value);
 	UFUNCTION(BlueprintCallable, Category = "QiyiDataManager")
		void Request_DeepLinkParam(); 
	UFUNCTION(BlueprintCallable, Category = "QiyiDataManager")
		void Request_LaunchHome();


public:
	//Init SDK
	void Request_InitQiyuSDK(); 

	//callback_function
	static void MessageProcess(uint64 messageID, const char* result, int32 resultSize);
	static void ListenerProcess(uint64 messageID, const char* result, int32 resultSize);

	//register
	void RegisterListener();

	//AddRequest
	//*request
	int32 AddRequest(FRequestCallback* rcb);
	//*listener
	void AddListener(EQIYIMessageCode messageCode, FRequestCallback* rcb);

	//IndexRequestID
	uint64 GetIndexRequestID()
	{
		return  ++indexRequestID;
	}

private:
	FQIYIMessageResult<FQIYISDKInit> s_QIYISDKInit;
	FQIYIAccountInfo s_QIYIAccountInfo;
	FQIYIAppUpdateInfo s_QIYIAppUpdateInfo;
	FQIYIDeepLinkParam s_QIYIDeepLinkParam;


private:
	TMap<uint64, FRequestCallback*> m_requestListMap;
	TMap<int32, FRequestCallback*> m_listenerListMap;

private:
	static UQIYIVRDataManager* s_QIYIVRDataManager;

	uint64 indexRequestID;

	FString temp;
	int32 b_isAccountLogin;
};
