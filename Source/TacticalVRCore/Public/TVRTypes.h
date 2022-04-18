// This file is covered by the LICENSE file in the root of this plugin.

#pragma once
#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ETVRLeftRight : uint8
{
	None,
	Left,
	Right
};

struct FTVRHysteresisValue
{
	FTVRHysteresisValue(const float Min = 0.5f, const float Max = 0.5f)
	{
		ThresholdMin = Min;
		ThresholdMax = Max;
		bState = false;
	}

	float ThresholdMin;
	float ThresholdMax;
	bool bState;

	bool UpdateValue(const float NewValue)
	{
		if(bState)
		{
			if(NewValue < ThresholdMin)
			{
				bState = false;
			}
		}
		else
		{
			if(NewValue > ThresholdMax)
			{
				bState = true;
			}
		}
		return bState;
	}
};
