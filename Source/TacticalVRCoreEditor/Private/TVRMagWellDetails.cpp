#include "TVRMagWellDetails.h"
#include "ContentBrowserModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IContentBrowserSingleton.h"
#include "PropertyCustomizationHelpers.h"

#include "Weapon/TVRMagazine.h"
#include "Weapon/Component/TVRMagazineWell.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "TVRMagWellDetails"

FTVRMagWellDetails::FTVRMagWellDetails()
{
}

void FTVRMagWellDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{	
	IDetailCategoryBuilder& Cat = DetailBuilder.EditCategory(TEXT("Magazine"));
	TSharedRef<IPropertyHandle> MagClassProp = DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(UTVRMagazineWell, MagazineClass));

	if(MagClassProp->IsValidHandle())
	{
		// only allow the custom input, that limits selection to the allowed classes
		DetailBuilder.HideProperty(MagClassProp);
	}
	
	TArray< TWeakObjectPtr< UObject > > Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);	
	if(Objects.Num() != 1)
	{
		// we do not allow multi object editing
		return;
	}	
	TWeakObjectPtr<UTVRMagazineWell> MagWell = Cast<UTVRMagazineWell>(Objects[0].Get());

	if(MagClassProp->IsValidHandle())
	{
		PopulateMagazineArray(MagWell.Get());

		FSimpleDelegate BrowseDelegate;
		BrowseDelegate.BindLambda(
			[MagClassProp]()
			{
				FAssetData AssetData;
				const auto Result = MagClassProp->GetValue(AssetData);
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
			[MagClassProp]()
			{
				MagClassProp->SetValueFromFormattedString(TEXT("None"));
			}
		);

		auto GenerateAllowedAttachmentList = [](TSharedPtr<TSubclassOf<ATVRMagazine>> MagClass)
		{
			FText DisplayName;
			if(MagClass.IsValid())
			{
				const auto ClassAsset = FAssetData(*MagClass.Get());
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

		auto SelectedAttachmentChanged = [MagClassProp](TSharedPtr<TSubclassOf<ATVRMagazine>> NewValue, ESelectInfo::Type SelectType)
		{
			if(NewValue.IsValid())
			{
				const FString ClassPath = GetPathNameSafe(*NewValue.Get());
				MagClassProp->SetValueFromFormattedString(ClassPath);
			}
			else
			{
				MagClassProp->ResetToDefault();
			}
		};

		auto CurrentAttachmentText = [MagClassProp]()
		{
			FText DisplayText;
			FAssetData AssetData;
			const auto Result = MagClassProp->GetValue(AssetData);
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
			.Text(NSLOCTEXT(LOCTEXT_NAMESPACE, "CurrentMagSelector", "Magazine Class"))
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
				SNew(SComboBox<TSharedPtr<TSubclassOf<ATVRMagazine>>>)
				.OptionsSource(&MagazineArray)
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

void FTVRMagWellDetails::PopulateMagazineArray(UTVRMagazineWell* MagWell)
{
	MagazineArray.Empty();
	TArray<TSubclassOf<ATVRMagazine>> AllowedMags;
	MagWell->GetAllowedMagazines(AllowedMags);
	for(auto LoopMag : AllowedMags)
	{				
		auto TempMag = MakeShareable(new TSubclassOf<ATVRMagazine>);
		*TempMag.Object = LoopMag;
		MagazineArray.Add(TempMag);
	}
}
