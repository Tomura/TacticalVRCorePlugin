// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "ContentBrowserModule.h"
#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IContentBrowserSingleton.h"
#include "PropertyCustomizationHelpers.h"

#include "Weapon/Attachments/TVRWeaponAttachment.h"
#include "Weapon/Attachments/WPNA_ForeGrip.h"
#include "Weapon/Attachments/WPNA_Light.h"
#include "Weapon/Attachments/WPNA_Muzzle.h"
#include "Weapon/Attachments/WPNA_PistolLight.h"
#include "Weapon/Attachments/WPNA_Laser.h"
#include "Weapon/Attachments/WPNA_Sight.h"
#include "Weapon/Attachments/WPNA_Stock.h"
#include "Weapon/Attachments/WPNA_Barrel.h"

#include "Weapon/Component/TVRAttachPoint_Laser.h"
#include "Weapon/Component/TVRAttachPoint_Light.h"
#include "Weapon/Component/TVRAttachPoint_Muzzle.h"
#include "Weapon/Component/TVRAttachPoint_PistolLight.h"
#include "Weapon/Component/TVRAttachPoint_Sight.h"
#include "Weapon/Component/TVRAttachPoint_Stock.h"
#include "Weapon/Component/TVRAttachPoint_Underbarrel.h"
#include "Weapon/Component/TVRAttachPoint_Barrel.h"


#define LOCTEXT_NAMESPACE "TVRAttachPointDetails"
class IDetailLayoutBuilder;

template<class AttachPointType, class AttachmentType>
class FTVRAttachPointDetails : public IDetailCustomization
{
public:
	FTVRAttachPointDetails(const FString& AttachmentProp)
	{
		AttachmentPropName = AttachmentProp;
	}
	
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance(const FString& AttachmentProp)
	{
		return MakeShareable(new FTVRAttachPointDetails<AttachPointType, AttachmentType>(AttachmentProp));
	}
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override
	{
		IDetailCategoryBuilder& Cat = DetailBuilder.EditCategory(TEXT("Attach Point"));
		TSharedRef<IPropertyHandle> CurrentAttachmentProp = DetailBuilder.GetProperty(*AttachmentPropName);
		if(CurrentAttachmentProp->IsValidHandle())
		{
			// only allow the custom input, that limits selection to the allowed classes
			DetailBuilder.HideProperty(CurrentAttachmentProp);
		}
	
		TArray< TWeakObjectPtr< UObject > > Objects;
		DetailBuilder.GetObjectsBeingCustomized(Objects);	
		if(Objects.Num() != 1)
		{
			// we do not allow multi object editing
			return;
		}	
		TWeakObjectPtr<AttachPointType> MyAttachmentPoint = Cast<AttachPointType>(Objects[0].Get());
		
		if(CurrentAttachmentProp->IsValidHandle())
		{		
			AttachmentsArray.Empty();
			TArray<TSubclassOf<ATVRWeaponAttachment>> AllowedAttachments;
			MyAttachmentPoint->GetAllowedAttachments(AllowedAttachments);
			for(auto LoopAttachment : AllowedAttachments)
			{
				if(LoopAttachment == nullptr || LoopAttachment->IsChildOf(AttachmentType::StaticClass()))
				{				
					auto TempSight = MakeShareable(new TSubclassOf<AttachmentType>);
					*TempSight.Object = LoopAttachment;
					AttachmentsArray.Add(TempSight);
				}
			}


			FSimpleDelegate BrowseDelegate;
			BrowseDelegate.BindLambda(
				[CurrentAttachmentProp]()
				{
					FAssetData AssetData;
					const auto Result = CurrentAttachmentProp->GetValue(AssetData);
					if(Result == FPropertyAccess::Result::Success)
					{
						const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
						TArray<FAssetData> AssetList;
						AssetList.Add(AssetData);
						ContentBrowserModule.Get().SyncBrowserToAssets(AssetList);
					}
					// return FReply::Handled();
				}
			);

			FSimpleDelegate ClearDelegate;
			ClearDelegate.BindLambda(
				[CurrentAttachmentProp]()
				{
					CurrentAttachmentProp->SetValueFromFormattedString(TEXT("None"));
				}
			);

			auto GenerateAllowedAttachmentList = [](TSharedPtr<TSubclassOf<AttachmentType>> AttachmentClass)
			{
				FText DisplayName;
				if(AttachmentClass.IsValid())
				{
					const auto ClassAsset = FAssetData(*AttachmentClass.Get());
					DisplayName = FText::FromName(ClassAsset.AssetName);
				}
				else
				{
					DisplayName = NSLOCTEXT(LOCTEXT_NAMESPACE,"Invalid Pointer","Invalid Pointer");
				}
				return SNew(STextBlock)
				.TextStyle(FEditorStyle::Get(), "NormalText")
				.Margin(2.f)
				.Text(DisplayName);
			};

			auto SelectedAttachmentChanged = [CurrentAttachmentProp](TSharedPtr<TSubclassOf<AttachmentType>> NewValue, ESelectInfo::Type SelectType)
			{
				if(NewValue.IsValid())
				{
					const FString ClassPath = GetPathNameSafe(*NewValue.Get());
					CurrentAttachmentProp->SetValueFromFormattedString(ClassPath);
				}
				else
				{
					CurrentAttachmentProp->ResetToDefault();
				}
			};

			auto CurrentAttachmentText = [CurrentAttachmentProp]()
			{
				FText DisplayText;
				FAssetData AssetData;
				const auto Result = CurrentAttachmentProp->GetValue(AssetData);
				if(Result == FPropertyAccess::Result::Success)
				{
					return FText::FromName(AssetData.AssetName);
				}
				return NSLOCTEXT(LOCTEXT_NAMESPACE, "InvalidDisplayText", "Invalid Display Text");
			};
			
			Cat.AddCustomRow(NSLOCTEXT(LOCTEXT_NAMESPACE, "CurrentSightSelector", "Current Attachment"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "CurrentSightSelector", "Current Attachment"))
				.TextStyle(FEditorStyle::Get(), "SmallText")
			]
			.ValueContent()
			.MaxDesiredWidth(800.f)
			.MinDesiredWidth(400.f)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SComboBox<TSharedPtr<TSubclassOf<AttachmentType>>>)
					.OptionsSource(
						&AttachmentsArray
					)
					.OnGenerateWidget_Lambda(GenerateAllowedAttachmentList)
					.OnSelectionChanged_Lambda(SelectedAttachmentChanged)
					[
						SNew(STextBlock)
						.TextStyle(FEditorStyle::Get(), "SmallText")
						.Text_Lambda(CurrentAttachmentText)
					]
				]
				+SHorizontalBox::Slot()
				.MaxWidth(20.f)
				.Padding(2.f)
				[
					PropertyCustomizationHelpers::MakeBrowseButton(BrowseDelegate)
				]
				+SHorizontalBox::Slot()
				.MaxWidth(20.f)
				.Padding(2.f)
				[
					PropertyCustomizationHelpers::MakeClearButton(ClearDelegate)
				]
			];
		}
	}

	FString AttachmentPropName;
	TArray<TSharedPtr<TSubclassOf<AttachmentType>>> AttachmentsArray;
};
#undef LOCTEXT_NAMESPACE


class FTVRAttachPointUnderbarrelDetails : public FTVRAttachPointDetails<UTVRAttachPoint_Underbarrel, AWPNA_ForeGrip>
{
public:
	FTVRAttachPointUnderbarrelDetails() : FTVRAttachPointDetails<UTVRAttachPoint_Underbarrel, AWPNA_ForeGrip>(FString("CurrentAttachmentClass"))
	{
	}
	
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FTVRAttachPointUnderbarrelDetails());
	}
};


class FTVRAttachPointLaserDetails : public FTVRAttachPointDetails<UTVRAttachPoint_Laser, AWPNA_Laser>
{
public:
	FTVRAttachPointLaserDetails() : FTVRAttachPointDetails<UTVRAttachPoint_Laser, AWPNA_Laser>(FString("CurrentLaserClass"))
	{
	}
	
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FTVRAttachPointLaserDetails());
	}
};


class FTVRAttachPointLightDetails : public FTVRAttachPointDetails<UTVRAttachPoint_Light, AWPNA_Light>
{
public:
	FTVRAttachPointLightDetails() : FTVRAttachPointDetails<UTVRAttachPoint_Light, AWPNA_Light>(FString("CurrentLightClass"))
	{
	}
	
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FTVRAttachPointLightDetails());
	}
};


class FTVRAttachPointPistolLightDetails : public FTVRAttachPointDetails<UTVRAttachPoint_PistolLight, AWPNA_PistolLight>
{
public:
	FTVRAttachPointPistolLightDetails() : FTVRAttachPointDetails<UTVRAttachPoint_PistolLight, AWPNA_PistolLight>(FString("CurrentLightClass"))
	{
	}
	
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FTVRAttachPointPistolLightDetails());
	}
};

class FTVRAttachPointSightDetails : public FTVRAttachPointDetails<UTVRAttachPoint_Sight, AWPNA_Sight>
{
public:
	FTVRAttachPointSightDetails() : FTVRAttachPointDetails<UTVRAttachPoint_Sight, AWPNA_Sight>(FString("CurrentSightClass"))
	{
	}
	
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FTVRAttachPointSightDetails());
	}
};


class FTVRAttachPointMuzzleDetails : public FTVRAttachPointDetails<UTVRAttachPoint_Muzzle, AWPNA_Muzzle>
{
public:
	FTVRAttachPointMuzzleDetails() : FTVRAttachPointDetails<UTVRAttachPoint_Muzzle, AWPNA_Muzzle>(FString("CurrentAttachmentClass"))
	{
	}
	
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FTVRAttachPointMuzzleDetails());
	}
};

class FTVRAttachPointStockDetails : public FTVRAttachPointDetails<UTVRAttachPoint_Stock, AWPNA_Stock>
{
public:
	FTVRAttachPointStockDetails() : FTVRAttachPointDetails<UTVRAttachPoint_Stock, AWPNA_Stock>(FString("CurrentAttachmentClass"))
	{
	}
	
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FTVRAttachPointStockDetails());
	}
};

class FTVRAttachPointBarrelDetails : public FTVRAttachPointDetails<UTVRAttachPoint_Barrel, AWPNA_Barrel>
{
public:
	FTVRAttachPointBarrelDetails() : FTVRAttachPointDetails<UTVRAttachPoint_Barrel, AWPNA_Barrel>(FString("CurrentAttachmentClass"))
	{
	}
	
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FTVRAttachPointBarrelDetails());
	}
};