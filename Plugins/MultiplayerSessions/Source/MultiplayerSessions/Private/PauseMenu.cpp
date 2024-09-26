// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenu.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Options.h"

void UPauseMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

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

bool UPauseMenu::Initialize()
{
	if (!Super::Initialize()) return false;

	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddDynamic(this, &ThisClass::ResumeButtonPressed);
	}
	
	if (OptionsButton)
	{
		OptionsButton->OnClicked.AddDynamic(this, &ThisClass::OptionsButtonPressed);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &ThisClass::QuitButtonPressed);
	}

	return true;
}

void UPauseMenu::NativeDestruct()
{
	MenuTeardown();
	Super::NativeDestruct();
}

void UPauseMenu::ResumeButtonPressed()
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

void UPauseMenu::OptionsButtonPressed()
{
}

void UPauseMenu::QuitButtonPressed()
{
}

void UPauseMenu::MenuTeardown()
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
