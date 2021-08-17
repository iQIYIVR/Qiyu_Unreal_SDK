#pragma once
#include "QiyuSettings.generated.h"

UENUM()
enum class EFFRSettings : uint8
{
	None,
	Low,
	Med,
	High
};

UCLASS(config = QIYUConfig, defaultconfig)
class  UQiyuSettings  : public UObject
{
	GENERATED_BODY()
public:
	UQiyuSettings(const FObjectInitializer& ObjectInitializer);

public:

	bool foveationFeature;
	float lowFoveationMinimum;

	UPROPERTY(Config, EditAnywhere, Category = "SDK", Meta = (DisplayName = "Enable FoveationEnabled?", ToolTip = "If you want to get Foveationstate, Please enable it!"))
		bool bFoveationEnabled;

	UPROPERTY(Config, EditAnywhere, Category = "SDK", Meta = (DisplayName = "FoveationLevel", ToolTip = "If you want to get Foveationstate, Please enable it!"))
		EFFRSettings FFREnum;

	UPROPERTY(Config, EditAnywhere, Category = "SDK", Meta = (DisplayName = "FoveationArea", ToolTip = "If you want to get Foveationstate, Please enable it!"))
		float FoveationArea;

	UPROPERTY(Config, EditAnywhere, Category = "SDK", Meta = (DisplayName = "FoveationGain", ToolTip = "If you want to get Foveationstate, Please enable it!"))
		FVector2D FoveationGain;

	UPROPERTY(Config, EditAnywhere, Category = "Platform", Meta = (DisplayName = "appID"))
		FString appID;
	UPROPERTY(Config, EditAnywhere, Category = "Platform", Meta = (DisplayName = "developer_id"))
		FString developer_id;
	UPROPERTY(Config, EditAnywhere, Category = "Platform", Meta = (DisplayName = "app_secret"))
		FString app_secret;
	UPROPERTY(Config, EditAnywhere, Category = "Platform", Meta = (DisplayName = "sign_key"))
		FString sign_key;


#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	void SetFFRValue();
};