// Fill out your copyright notice in the Description page of Project Settings.


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

	InitializeResolutionComboBox();
	InitializeVSync();
	InitializeFramerate();

	const FSelectionElement SelectionElements[] = {
		{ ShadingQualitySelection, &UGameUserSettings::GetShadingQuality, &UGameUserSettings::SetShadingQuality },
		{ GlobalIlluminationQualitySelection, &UGameUserSettings::GetGlobalIlluminationQuality, &UGameUserSettings::SetGlobalIlluminationQuality },
		{ PostProcessingQualitySelection, &UGameUserSettings::GetPostProcessingQuality, &UGameUserSettings::SetPostProcessingQuality },
		{ VisualEffectsQualitySelection, &UGameUserSettings::GetVisualEffectQuality, &UGameUserSettings::SetVisualEffectQuality },
		{ ShadowQualittSelection, &UGameUserSettings::GetShadowQuality, &UGameUserSettings::SetShadowQuality }
	};

	for (const auto& [Widget, GetFunc, SetFunc] : SelectionElements)
	{
		const auto CurrentSelection = std::invoke(GetFunc, GameUserSettings);
		Widget->SetCurrentSelection(CurrentSelection);
		Widget->OnSelectionChange.BindLambda([this, SetFunc](int InSelection)
		{
				std::invoke(SetFunc, GameUserSettings, InSelection);
				GameUserSettings->ApplySettings(false);
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
	/*
	
	if (ResolutionLeftButton)
	{
		ResolutionLeftButton->OnClicked.AddDynamic(this, &ThisClass::ResolutionLeftButtonClicked);
	}
	if (ResolutionRightButton)
	{
		ResolutionRightButton->OnClicked.AddDynamic(this, &ThisClass::ResolutionRightButtonClicked);
	}
	if (GraphicsLeftButton)
	{
		GraphicsLeftButton->OnClicked.AddDynamic(this, &ThisClass::GraphicsLeftButtonClicked);
	}
	if (GraphicsRightButton)
	{
		GraphicsRightButton->OnClicked.AddDynamic(this, &ThisClass::GraphicsRightButtonClicked);
	}
	if (VSyncLeftButton)
	{
		VSyncLeftButton->OnClicked.AddDynamic(this, &ThisClass::VSyncLeftButtonClicked);
	}
	if (VSyncRightButton)
	{
		VSyncRightButton->OnClicked.AddDynamic(this, &ThisClass::VSyncRightButtonClicked);
	}
	
	*/
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &ThisClass::BackButtonClicked);
	}
	
	return true;
}

void UOptions::NativeDestruct()
{
	MenuTeardown();
	Super::NativeDestruct();
}

void UOptions::InitializeResolutionComboBox()
{
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
			GameUserSettings->ApplySettings(false);
	});
}

void UOptions::OnResolutionChanged(FString InSelectedItem, ESelectInfo::Type InSelectionType)
{
	const auto SelectedResolution = Resolutions[ResolutionComboBox->GetSelectedIndex()];
	GameUserSettings->SetScreenResolution(SelectedResolution);
	GameUserSettings->ApplySettings(false);
}

void UOptions::OnVSyncChanged(bool InIsChecked)
{
	GameUserSettings->SetVSyncEnabled(InIsChecked);
	GameUserSettings->ApplySettings(false);
}

void UOptions::GameplayButtonClicked()
{
	SwitcherIndex = 0;
	UpdateSwitcherPanel(SwitcherIndex);
}

void UOptions::ControllerButtonClicked()
{
	SwitcherIndex = 1;
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
	SwitcherIndex = 5;
	UpdateSwitcherPanel(SwitcherIndex);
}

void UOptions::WindowLeftButtonClicked()
{
	int32 NewWindowMode = FMath::Clamp(WindowMode, 0, 2);
	NewWindowMode = (WindowMode - 1);
	WindowMode = NewWindowMode;
}

void UOptions::WindowRightButtonClicked()
{

}

void UOptions::ResolutionLeftButtonClicked()
{
}

void UOptions::ResolutionRightButtonClicked()
{
}

void UOptions::GraphicsLeftButtonClicked()
{
}

void UOptions::GraphicsRightButtonClicked()
{
}

void UOptions::VSyncLeftButtonClicked()
{
}

void UOptions::VSyncRightButtonClicked()
{
}

void UOptions::BackButtonClicked()
{
	RemoveFromParent();
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeGameOnly InputMode;
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(false);
		}
	}
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
