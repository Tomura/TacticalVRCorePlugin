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
	TSharedRef<IPropertyHandle> RecompileHelperProp = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ATVRGunBase, bForceRecompile));
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
	
	for(const auto AttachPoint : AttachPoints)
	{
		if(AttachPoint)
		{
			AddAttachmentRow(Cat, AttachPoint, MyGun.Get(), RecompileHelperProp);
			// we'll look for child attachments.
			// This is limited to a depth of one. everything else is probably unreasonable and in that case it might be
			// better to actually solve that in-game.
			const auto ChildAttachment = AttachPoint->GetCurrentAttachment();
			if(ChildAttachment)
			{
				TArray<UTVRAttachmentPoint*> SubAttachPoints;
				ChildAttachment->GetComponents<UTVRAttachmentPoint>(SubAttachPoints);
				for(const auto LoopSubAttachPoint: SubAttachPoints)
				{
					if(LoopSubAttachPoint)
					{
						AddSubAttachmentRow(Cat, LoopSubAttachPoint, AttachPoint, MyGun.Get(), RecompileHelperProp);
					}
				}
			}
		}
	}
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

UTVRAttachmentPoint* FTVRGunDetails::GetAttachmentPointByName(const AActor* Parent, FName AttachPointName) const
{
	TArray<UTVRAttachmentPoint*> AllAttachPoints;
	Parent->GetComponents<UTVRAttachmentPoint>(AllAttachPoints);
	for(const auto& TestPoint: AllAttachPoints)
	{
		if(TestPoint->GetFName() == AttachPointName)
		{
			return TestPoint;
		}
	}
	return nullptr;
}

void FTVRGunDetails::AddAttachmentRow(IDetailCategoryBuilder& Cat, UTVRAttachmentPoint* AttachPoint, ATVRGunBase* Gun, TSharedRef<IPropertyHandle> RecompileHelperProp)
{
	const auto AttachPointName = AttachPoint->GetFName();
	const int32 i = PopulateAttachmentsArrayFor(AttachPoint);
	
	FSimpleDelegate BrowseDelegate;
	BrowseDelegate.BindLambda(
		[this, Gun, AttachPointName]()
		{
			if(const auto AttachPoint = GetAttachmentPointByName(Gun, AttachPointName))
			{
				FAssetData AssetData = FAssetData(AttachPoint->GetCurrentAttachmentClass());
				const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
				TArray<FAssetData> AssetList;
				AssetList.Add(AssetData);
				ContentBrowserModule.Get().SyncBrowserToAssets(AssetList);
			}
		}
	);

	FSimpleDelegate ClearDelegate;
	ClearDelegate.BindLambda(
		[this, Gun, AttachPointName, RecompileHelperProp]()
		{
			if(const auto AttachPoint = GetAttachmentPointByName(Gun, AttachPointName))
			{		
				AttachPoint->SetCurrentAttachmentClass(nullptr);
			
				FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
				PropertyEditorModule.NotifyCustomizationModuleChanged();			
				RecompileHelperProp->SetValue(true);
			}
		}
	);
	Cat.AddCustomRow(FText::FromString(AttachPoint->GetName()))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString(AttachPoint->GetName()))
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
				[this, Gun, AttachPointName, RecompileHelperProp](AttachmentClassPtr NewValue, ESelectInfo::Type SelectType)
				{
					if(const auto AttachPoint = GetAttachmentPointByName(Gun, AttachPointName))
					{
						AttachPoint->SetCurrentAttachmentClass(*NewValue);
						// DetailBuilder.ForceRefreshDetails();
						FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
						PropertyEditorModule.NotifyCustomizationModuleChanged();
						RecompileHelperProp->SetValue(true);
					}
				}
			)
			[
				SNew(STextBlock)
				.TextStyle(FEditorStyle::Get(), "SmallText")
				.Text_Lambda(
					[this, Gun, AttachPointName]()
					{
						if(const auto AttachPoint = GetAttachmentPointByName(Gun, AttachPointName))
						{
							const auto CurrentAttachment = AttachPoint->GetCurrentAttachmentClass();
							const auto ClassAsset = FAssetData(CurrentAttachment);
							return FText::FromName(ClassAsset.AssetName);
						}
						return NSLOCTEXT(LOCTEXT_NAMESPACE, "AttachmentRefInvalid", "Invalid Atachment Ref");
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

void FTVRGunDetails::AddSubAttachmentRow(IDetailCategoryBuilder& Cat, UTVRAttachmentPoint* AttachPoint,
	UTVRAttachmentPoint* ParentPoint, ATVRGunBase* Gun, TSharedRef<IPropertyHandle> RecompileHelperProp)
{
	const auto AttachPointName = AttachPoint->GetFName();
	const auto ParentPointName = ParentPoint->GetFName();
	const int32 i = PopulateAttachmentsArrayFor(AttachPoint);
	
	FSimpleDelegate BrowseDelegate;
	BrowseDelegate.BindLambda(
		[this, Gun, ParentPointName, AttachPointName]()
		{
			if(const auto ParentPoint = GetAttachmentPointByName(Gun, ParentPointName))
			{
				if(const auto ParentAttachment = ParentPoint->GetCurrentAttachment())
				{				
					if(const auto AttachPoint = GetAttachmentPointByName(ParentAttachment, AttachPointName))
					{
						FAssetData AssetData = FAssetData(AttachPoint->GetCurrentAttachmentClass());
						const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
						TArray<FAssetData> AssetList;
						AssetList.Add(AssetData);
						ContentBrowserModule.Get().SyncBrowserToAssets(AssetList);
					}
				}
			}
		}
	);

	FSimpleDelegate ClearDelegate;
	ClearDelegate.BindLambda(
		[this, Gun, ParentPointName, AttachPointName, RecompileHelperProp]()
		{
			if(const auto ParentPoint = GetAttachmentPointByName(Gun, ParentPointName))
			{
				if(const auto ParentAttachment = ParentPoint->GetCurrentAttachment())
				{
					if(const auto AttachPoint = GetAttachmentPointByName(ParentAttachment, AttachPointName))
					{					
						AttachPoint->SetCurrentAttachmentClass(nullptr);				
						FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
						PropertyEditorModule.NotifyCustomizationModuleChanged();
						RecompileHelperProp->SetValue(true);
					}
				}
			}
		}
	);
	Cat.AddCustomRow(FText::FromString(AttachPoint->GetName()))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString(FString(TEXT("-- ")).Append(AttachPoint->GetName())))
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
				[this, Gun, AttachPointName, ParentPointName, RecompileHelperProp](AttachmentClassPtr NewValue, ESelectInfo::Type SelectType)
				{
					if(const auto ParentPoint = GetAttachmentPointByName(Gun, ParentPointName))
					{
						if(const auto ParentAttachment = ParentPoint->GetCurrentAttachment())
						{
							if(const auto AttachPoint = GetAttachmentPointByName(ParentAttachment, AttachPointName))
							{							
								AttachPoint->SetCurrentAttachmentClass(*NewValue);
								// DetailBuilder.ForceRefreshDetails();
								FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
								PropertyEditorModule.NotifyCustomizationModuleChanged();
								RecompileHelperProp->SetValue(true);
							}
						}
					}
				}
			)
			[
				SNew(STextBlock)
				.TextStyle(FEditorStyle::Get(), "SmallText")
				.Text_Lambda(
					[this, Gun, AttachPointName, ParentPointName]()
					{
						if(const auto ParentPoint = GetAttachmentPointByName(Gun, ParentPointName))
						{
							if(const auto ParentAttachment = ParentPoint->GetCurrentAttachment())
							{
								const auto AttachPoint = GetAttachmentPointByName(ParentAttachment, AttachPointName);
								const auto CurrentAttachment = AttachPoint->GetCurrentAttachmentClass();
								const auto ClassAsset = FAssetData(CurrentAttachment);
								return FText::FromName(ClassAsset.AssetName);
							}
						}
						return NSLOCTEXT(LOCTEXT_NAMESPACE,"Invalid Pointer","Invalid Pointer");
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
