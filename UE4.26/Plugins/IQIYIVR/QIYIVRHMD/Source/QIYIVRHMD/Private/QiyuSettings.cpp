
#include "QiyuSettings.h"

UQiyuSettings::UQiyuSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	foveationFeature(true),
	bFoveationEnabled(true),
	lowFoveationMinimum(0.125f),
	FFREnum(EFFRSettings::None),
	FoveationArea(1),
	FoveationGain(3,3),
	appID(TEXT("")),
	developer_id(TEXT("")),
	app_secret(TEXT("")),
	sign_key(TEXT(""))
{
}

#if WITH_EDITOR
void UQiyuSettings::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	Super::UpdateDefaultConfigFile();

	//设置FFR
	SetFFRValue();
}
#endif // #if WITH_EDITOR

void UQiyuSettings::SetFFRValue()
{
	if (bFoveationEnabled)
	{
		//设置FFR默认值
		switch (FFREnum)
		{
		case EFFRSettings::None:
			break;
		case EFFRSettings::Low:
			FoveationArea = 1.0f;
			FoveationGain.Set(3.3f, 3.3f);
			break;
		case EFFRSettings::Med:
			FoveationArea = 1.0f;
			FoveationGain.Set(4.4f, 4.4f);
			break;
		case EFFRSettings::High:
			FoveationArea = 1.0f;
			FoveationGain.Set(6.6f, 6.6f);
			break;
		default:
			break;
		}
	}
	else
	{
		FFREnum = EFFRSettings::None;
		FoveationArea = 0.0f;
		FoveationGain.Set(0.0f, 0.0f);
	}
}

