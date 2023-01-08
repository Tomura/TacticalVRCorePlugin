#pragma once

#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"

class FTVRMagWellDetails : public IDetailCustomization
{
public:
	FTVRMagWellDetails();
		
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FTVRMagWellDetails());
	}
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	virtual void PopulateMagazineArray(class UTVRMagazineWell* MagWell);
	
	TArray<TSharedPtr<TSubclassOf<class ATVRMagazine>>> MagazineArray;
};
