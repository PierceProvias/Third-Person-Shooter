#include "Options.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Components/WidgetSwitcher.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "SelectionBase.h"
#include "Framerates.h"
//#include "GameSettingCollection.h"
//#include "GameSetting.h"
//#include "Engine/LocalPlayer.h"
#include <functional>
//#include <iostream>

// Anonymous namespace used only in this implementation file and prevents it from leaking to other files
namespace
{
	constexpr EFramerate FramerateOptions[] = {
			EFramerate::FPS_30,
			EFramerate::FPS_60,
			EFramerate::FPS_120,
			EFramerate::FPS_240,
			EFramerate::FPS_Uncapped
	};

	typedef int32(UGameUserSettings::*GetFunc)() const;
	typedef void(UGameUserSettings::*SetFunc)(int);

	struct FSelectionElement
	{
		USelectionBase* Widget;
		GetFunc GetFunc;
		SetFunc SetFunc;
	};
}

void UOptions::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;
	WidgetSwitcher->ActiveWidgetIndex = 0;

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeUIOnly InputModeData; // Focused only on UI
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
			PlayerController->SetInputMode(InputModeData);
	
			PlayerController->SetShowMouseCursor(true);
		}
	}
}

void UOptions::NativeConstruct()
{
	GameUserSettings = UGameUserSettings::GetGameUserSettings();

	//InitializeWindowMode();
	InitializeResolutionComboBox();
	InitializeVSync();
	InitializeFramerate();

	const FSelectionElement SelectionElements[] = {
	
		//{ WindowModeSelection, &UGameUserSettings::GetFullscreenMode, &UGameUserSettings::SetFullscreenMode},
		{ ShadingQualitySelection,				&UGameUserSettings::GetShadingQuality,				&UGameUserSettings::SetShadingQuality },
		{ GlobalIlluminationQualitySelection,	&UGameUserSettings::GetGlobalIlluminationQuality,	&UGameUserSettings::SetGlobalIlluminationQuality },
		{ PostProcessingQualitySelection,		&UGameUserSettings::GetPostProcessingQuality,		&UGameUserSettings::SetPostProcessingQuality },
		{ VisualEffectsQualitySelection,		&UGameUserSettings::GetVisualEffectQuality,			&UGameUserSettings::SetVisualEffectQuality },
		{ ShadowQualitySelection,				&UGameUserSettings::GetShadowQuality,				&UGameUserSettings::SetShadowQuality },
		{ AntiAliasingQualitySelection,			&UGameUserSettings::GetAntiAliasingQuality,			&UGameUserSettings::SetAntiAliasingQuality },
		{ ReflectionQualitySelection,			&UGameUserSettings::GetReflectionQuality,			&UGameUserSettings::SetReflectionQuality },
		{ TextureQualitySelection,				&UGameUserSettings::GetTextureQuality,				&UGameUserSettings::SetTextureQuality },
		{ FoliageQualitySelection,				&UGameUserSettings::GetFoliageQuality,				&UGameUserSettings::SetFoliageQuality }

	};

	for (const auto& [Widget, GetFunc, SetFunc] : SelectionElements)
	{
		const auto CurrentSelection = std::invoke(GetFunc, GameUserSettings);
		Widget->SetCurrentSelection(CurrentSelection);
		Widget->OnSelectionChange.BindLambda([this, SetFunc](int InSelection)
		{
				std::invoke(SetFunc, GameUserSettings, InSelection);
				//GameUserSettings->ApplySettings(false);
		});
	}
}

bool UOptions::Initialize()
{
	if (!Super::Initialize()) return false;

	if (GameplayButton)
	{
		GameplayButton->OnClicked.AddDynamic(this, &ThisClass::GameplayButtonClicked);
	}
	if (ControllerButton)
	{
		ControllerButton->OnClicked.AddDynamic(this, &ThisClass::ControllerButtonClicked);
	}
	if (MouseKeyboardButton)
	{
		MouseKeyboardButton->OnClicked.AddDynamic(this, &ThisClass::MouseKeyboardButtonClicked);
	}
	if (AccessibilityButton)
	{
		AccessibilityButton->OnClicked.AddDynamic(this, &ThisClass::AccessibilityButtonClicked);
	}	
	if (AudioButton)
	{
		AudioButton->OnClicked.AddDynamic(this, &ThisClass::AudioButtonClicked);
	}
	if (VideoButton)
	{
		VideoButton->OnClicked.AddDynamic(this, &ThisClass::VideoButtonClicked);
	}

	if (WindowLeftButton)
	{
		WindowLeftButton->OnClicked.AddDynamic(this, &ThisClass::WindowLeftButtonClicked); // Binds host button callback to delegate
	}
	if (WindowRightButton)
	{
		WindowRightButton->OnClicked.AddDynamic(this, &ThisClass::WindowRightButtonClicked);
	}

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &ThisClass::BackButtonClicked);
	}
	if (ApplyButton)
	{
		ApplyButton->OnClicked.AddDynamic(this, &ThisClass::ApplyButtonClicked);
	}
	if (RestoreButton)
	{
		RestoreButton->OnClicked.AddDynamic(this, &ThisClass::RestoreButtonClicked);
	}
	
	return true;
}

void UOptions::NativeDestruct()
{
	MenuTeardown();
	Super::NativeDestruct();
}

/*
	UGameSettingCollection* UOptions::InitializeVideoSettings(ULocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* Screen = NewObject<UGameSettingCollection>();
	Screen->SetDevName(TEXT("VideoCollection"));
	return nullptr;
}
*/

	
	/*
		FramerateSelection->OnSelectionChange.BindLambda([this](const int InSelection)
void UOptions::InitializeWindowMode()
{
	GameUserSettings->GetPreferredFullscreenMode();
	WindowModeSelection->Clear();
	int WindowModeOptionIndex = 0;
	const auto CurrentWindowModeOption = GameUserSettings->GetFullscreenMode();
	if (CurrentWindowModeOption != WindowModeSelection)
	{
		switch (WindowModeOptionIndex)
		{
		case EWindowMode::Fullscreen:
			GameUserSettings->SetFullscreenMode(EWindowMode::Fullscreen);
			break;
		case EWindowMode::WindowedFullscreen:
			GameUserSettings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
			break;
		case EWindowMode::Windowed:
			GameUserSettings->SetFullscreenMode(EWindowMode::Windowed);
			break;
		}
	}
			{
				GameUserSettings->SetFrameRateLimit(FFramerateUtils::EnumToValue(FramerateOptions[InSelection]));
				//GameUserSettings->ApplySettings(false);
				//GameUserSettings->ResetToCurrentSettings

			});
	WindowModeSelection->OnSelectionChange.BindLambda([this](const int InFullscreeMode)
		{
			GameUserSettings->SetFullscreenMode(EWindowMode::ConvertIntToWindowMode(InFullscreeMode));
		});
}
	*/


void UOptions::InitializeWindowMode()
{
	/*
	
	WindowModeSelection->Clear();
	WindowModeSelection->AddOption({ FText::FromString("Fullscreen") });
	WindowModeSelection->AddOption({ FText::FromString("Windowed Fullscreen") });
	WindowModeSelection->AddOption({ FText::FromString("Windowed") });

	const auto CurrentWindowMode = GameUserSettings->GetFullscreenMode();
	
	//WindowModeSelection->OnSelectionChange.AddDynamic(this, &UOptions::OnWindowModeChanged);
	*/

}

void UOptions::InitializeResolutionComboBox()
{
	// TODO: Fix combo box to display current resolution settings
	Resolutions.Reset();
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);

	ResolutionComboBox->ClearOptions();
	for (const auto& Resolution : Resolutions)
	{
		const auto ResolutionString = FString::Printf(TEXT("%dx%d"), Resolution.X, Resolution.Y);
		ResolutionComboBox->AddOption(ResolutionString);
	}

	// Find current resolution
	const auto CurrentResolution = GameUserSettings->GetScreenResolution();
	const auto SelectedIndex = Resolutions.IndexOfByPredicate([&CurrentResolution](const FIntPoint& InResolution)
		{
			return InResolution == CurrentResolution;
		});
	check(SelectedIndex >= 0);

	// Listen to changes
	ResolutionComboBox->SetDisplayLabel(GameUserSettings->GetScreenResolution().ToString());
	ResolutionComboBox->OnSelectionChanged.Clear();
	ResolutionComboBox->OnSelectionChanged.AddDynamic(this, &UOptions::OnResolutionChanged);
}

void UOptions::InitializeVSync()
{
	VSyncCheckBox->SetIsChecked(GameUserSettings->IsVSyncEnabled());
	VSyncCheckBox->OnCheckStateChanged.Clear();
	VSyncCheckBox->OnCheckStateChanged.AddDynamic(this, &UOptions::OnVSyncChanged);
}

void UOptions::InitializeFramerate()
{
	FramerateSelection->Clear();

	int FramerateOptionIndex = 0;
	
	const auto CurrentFramerate = GameUserSettings->GetFrameRateLimit();
	for (const auto& Framerate : FramerateOptions)
	{
		FramerateSelection->AddOption({

			FText::FromString(FFramerateUtils::EnumToString(Framerate))
		});

		if (CurrentFramerate == FFramerateUtils::EnumToValue(Framerate))
		{
			FramerateSelection->SetCurrentSelection(FramerateOptionIndex);
		}
		FramerateOptionIndex++;
	}
	FramerateSelection->OnSelectionChange.BindLambda([this](const int InSelection)
	{
			GameUserSettings->SetFrameRateLimit(FFramerateUtils::EnumToValue(FramerateOptions[InSelection]));
			//GameUserSettings->ApplySettings(false);
			//GameUserSettings->ResetToCurrentSettings
			
	});
}


void UOptions::OnWindowModeChanged(EWindowMode::Type InFullScreenMode)
{
	GameUserSettings->SetFullscreenMode(InFullScreenMode);
}




void UOptions::OnResolutionChanged(FString InSelectedItem, ESelectInfo::Type InSelectionType)
{
	const auto SelectedResolution = Resolutions[ResolutionComboBox->GetSelectedIndex()];
	GameUserSettings->SetScreenResolution(SelectedResolution);
	//GameUserSettings->ApplySettings(false);
}

void UOptions::OnVSyncChanged(bool InIsChecked)
{
	GameUserSettings->SetVSyncEnabled(InIsChecked);
	//GameUserSettings->ApplySettings(false);
}

void UOptions::GameplayButtonClicked()
{
	SwitcherIndex = 0;
	UpdateSwitcherPanel(SwitcherIndex);
}

void UOptions::ControllerButtonClicked()
{
	SwitcherIndex = 5;
	UpdateSwitcherPanel(SwitcherIndex);
}

void UOptions::MouseKeyboardButtonClicked()
{
	SwitcherIndex = 2;
	UpdateSwitcherPanel(SwitcherIndex);
}

void UOptions::AccessibilityButtonClicked()
{
	SwitcherIndex = 3;
	UpdateSwitcherPanel(SwitcherIndex);
}

void UOptions::AudioButtonClicked()
{
	SwitcherIndex = 4;
	UpdateSwitcherPanel(SwitcherIndex);
}

void UOptions::VideoButtonClicked()
{
	SwitcherIndex = 1;
	UpdateSwitcherPanel(SwitcherIndex);
}

void UOptions::WindowLeftButtonClicked()
{
	
}

void UOptions::WindowRightButtonClicked()
{

}

void UOptions::BackButtonClicked()
{
	RemoveFromParent();
	MenuTeardown();
}

void UOptions::ApplyButtonClicked()
{
	GameUserSettings->ApplySettings(false);
	//GameUserSettings->GetGameUserSettings()->ApplySettings(false);
}

void UOptions::RestoreButtonClicked()
{
	GameUserSettings->GetGameUserSettings()->ResetToCurrentSettings();
}

void UOptions::MenuTeardown()
{
	RemoveFromParent();
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeUIOnly InputMode;
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(true);
		}
	}
}

void UOptions::UpdateSwitcherPanel(int32 Index)
{

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			FColor::Yellow,
			FString::Printf(TEXT("UpdateSwitcherPanel Index: %d"), Index)
		);
	}

	if (WidgetSwitcher)
	{
		int32 CurrentIndex = WidgetSwitcher->GetActiveWidgetIndex();
		if (CurrentIndex != Index)
		{
			WidgetSwitcher->SetActiveWidgetIndex(Index);
		}

	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				3.f,
				FColor::Red,
				FString("Switcher not working!")
			);
		}
	}
}
