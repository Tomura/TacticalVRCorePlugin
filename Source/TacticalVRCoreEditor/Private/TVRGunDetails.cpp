// Copyright (c) 2020 Tammo Beil. All rights reserved.


#include "TVRGunDetails.h"

#include "ContentBrowserModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IContentBrowserSingleton.h"
#include "PropertyCustomizationHelpers.h"
#include "Weapon/TVRGunBase.h"
#include "Weapon/Attachments/TVRWeaponAttachment.h"
#include "Weapon/Component/TVRAttachmentPoint.h"

#define LOCTEXT_NAMESPACE "TVRGunDetails"

FTVRGunDetails::FTVRGunDetails()
{
	// currently there is nothing to do here
}

void FTVRGunDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Cat = DetailBuilder.EditCategory(TEXT("Weapon Attachments"));
	
	TArray< TWeakObjectPtr< UObject > > Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if(Objects.Num() != 1)
	{
		// we do not allow multi object editing
		return;
	}

	TWeakObjectPtr<ATVRGunBase> MyGun = Cast<ATVRGunBase>(Objects[0].Get());
	if(!MyGun.IsValid())
	{
		return;
	}

	TArray<UTVRAttachmentPoint*> AttachPoints;
	MyGun->GetComponents<UTVRAttachmentPoint>(AttachPoints);

	AttachmentsArray.Empty();
	
	for(auto AttachPoint : AttachPoints)
	{
		if(AttachPoint)
		{
			const int32 i = PopulateAttachmentsArrayFor(AttachPoint);
			
			FSimpleDelegate BrowseDelegate;
			BrowseDelegate.BindLambda(
				[AttachPoint]()
				{
					FAssetData AssetData = FAssetData(AttachPoint->GetCurrentAttachmentClass());
					const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
					TArray<FAssetData> AssetList;
					AssetList.Add(AssetData);
					ContentBrowserModule.Get().SyncBrowserToAssets(AssetList);
				}
			);

			FSimpleDelegate ClearDelegate;
			ClearDelegate.BindLambda(
				[AttachPoint]()
				{
					AttachPoint->SetCurrentAttachmentClass(nullptr);
				}
			);
			
			Cat.AddCustomRow(FText::FromString(AttachPoint->GetName()))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString(AttachPoint->GetName().Append(FString::FromInt(i))))
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
					SNew(SComboBox<AttachmentClassPtr>)
					.OptionsSource(
						AttachmentsArray[i].Get()
					)
					.OnGenerateWidget_Lambda(
						[](AttachmentClassPtr AttachmentClass)
						{
							FText DisplayName;
							if(AttachmentClass.IsValid())
							{
								const auto MyAttach = AttachmentClass.Get();
								const auto ClassAsset = FAssetData(*MyAttach);
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
						}
					)
					.OnSelectionChanged_Lambda(
						[AttachPoint](AttachmentClassPtr NewValue, ESelectInfo::Type SelectType)
						{
							AttachPoint->SetCurrentAttachmentClass(*NewValue);
						}
					)
					[
						SNew(STextBlock)
						.TextStyle(FEditorStyle::Get(), "SmallText")
						.Text_Lambda(
							[AttachPoint]()
							{
								FText DisplayText;
								const auto CurrentAttachment = AttachPoint->GetCurrentAttachmentClass();
								const auto ClassAsset = FAssetData(CurrentAttachment);
								return FText::FromName(ClassAsset.AssetName);
							}
						)
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

	UE_LOG(LogTemp, Log, TEXT("Finished GunDetails"));
}

int32 FTVRGunDetails::PopulateAttachmentsArrayFor(UTVRAttachmentPoint* AttachPoint)
{
	TArray<TSubclassOf<ATVRWeaponAttachment>> AllowedAttachments;
	AttachPoint->GetAllowedAttachments(AllowedAttachments);
	
	AttachmentClassArrayPtr SharedTempArray = MakeShareable(new TArray<AttachmentClassPtr>());
	for(const auto LoopAttachment : AllowedAttachments)
	{
		auto TempAttachment = MakeShareable(new TSubclassOf<ATVRWeaponAttachment>);
		*TempAttachment.Object = LoopAttachment;
		SharedTempArray.Get()->Add(TempAttachment);
	}
	const int32 i = AttachmentsArray.Add(SharedTempArray);
	return i;
}
