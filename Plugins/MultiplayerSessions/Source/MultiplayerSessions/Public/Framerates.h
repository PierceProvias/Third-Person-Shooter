#pragma once

//#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EFramerate : uint8
{
	FPS_30				UMETA(DisplayName = "30 FPS"),
	FPS_60				UMETA(DisplayName = "60 FPS"),
	FPS_120				UMETA(DisplayName = "120 FPS"),
	FPS_240				UMETA(DisplayName = "240 FPS"),

	FPS_Uncapped		UMETA(DisplayName = "Uncapped")
};

// Helper class 
class FFramerateUtils
{
public:

	static int EnumToValue(const EFramerate& InFrame)
	{
		switch (InFrame)
		{
		case EFramerate::FPS_30:			return 30;
		case EFramerate::FPS_60:			return 60;
		case EFramerate::FPS_120:			return 120;
		case EFramerate::FPS_240:			return 240;
		default:							return 0;
		}
	}

	static FString EnumToString(const EFramerate& InFrame)
	{
		const auto Value = EnumToValue(InFrame);
		return Value > 0 ? FString::Printf(TEXT("%d FPS"), Value) : FString::Printf(TEXT("Uncapped"));
	}
};