// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenu.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Options.h"
#include "Kismet/KismetSystemLibrary.h"

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
	MenuTeardown();
	RemoveFromParent();
}

void UPauseMenu::OptionsButtonPressed()
{

}

void UPauseMenu::QuitButtonPressed()
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = GetOwningPlayer();
	if (World && PlayerController)
	{
		UKismetSystemLibrary::QuitGame(
			World,
			PlayerController,
			EQuitPreference::Quit,
			true
		);
	}
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
