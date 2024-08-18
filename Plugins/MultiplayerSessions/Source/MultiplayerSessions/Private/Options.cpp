// Fill out your copyright notice in the Description page of Project Settings.


#include "Options.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Components/WidgetSwitcher.h"
#include "Math/UnrealMathUtility.h"

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
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &ThisClass::BackButtonClicked);
	}
	
	*/
	
	return true;
}

void UOptions::NativeDestruct()
{
	//MenuTeardown();
	//Super::NativeDestruct();
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
}

void UOptions::MenuTeardown()
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
