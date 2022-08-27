// Copyright (c) 2020 Tammo Beil. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"

/**
 * 
 */
class FTVRGunDetails : public IDetailCustomization
{
public:
	typedef TSharedPtr<TSubclassOf<class ATVRWeaponAttachment>> AttachmentClassPtr;
	typedef TSharedPtr<TArray<AttachmentClassPtr>> AttachmentClassArrayPtr;
	
	FTVRGunDetails();
	
	TArray<AttachmentClassArrayPtr> AttachmentsArray;
	
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FTVRGunDetails());
	}

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	virtual int32 PopulateAttachmentsArrayFor(class UTVRAttachmentPoint* AttachPoint);

	virtual class UTVRAttachmentPoint* GetAttachmentPointByName(const AActor* Parent, FName AttachPointName) const;

	virtual void AddAttachmentRow(IDetailCategoryBuilder& Cat, UTVRAttachmentPoint* AttachPoint, class ATVRGunBase* Gun, TSharedRef<IPropertyHandle> RecompileHelperProp);
	virtual void AddSubAttachmentRow(IDetailCategoryBuilder& Cat, UTVRAttachmentPoint* AttachPoint, UTVRAttachmentPoint* ParentPoint, class ATVRGunBase* Gun, TSharedRef<IPropertyHandle> RecompileHelperProp);
};
