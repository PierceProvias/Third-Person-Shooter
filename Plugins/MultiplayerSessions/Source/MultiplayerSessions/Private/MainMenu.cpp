// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Options.h"


void UMainMenu::MenuSetup()
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
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::LockInFullscreen);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
}

bool UMainMenu::Initialize()
{
	if (!Super::Initialize()) return false;
	
	if (PlayButton)
	{
		PlayButton->OnClicked.AddDynamic(this, &ThisClass::PlayButtonPressed);
	}
	if (CharacterButton)
	{
		CharacterButton->OnClicked.AddDynamic(this, &ThisClass::CharacterButtonPressed);
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


void UMainMenu::NativeDestruct()
{
//	MenuTeardown();
	//Super::NativeDestruct();
}

void UMainMenu::PlayButtonPressed()
{
}

void UMainMenu::CharacterButtonPressed()
{
}

void UMainMenu::OptionsButtonPressed()
{
	
}

void UMainMenu::QuitButtonPressed()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		PlayerController->ConsoleCommand(FString("quit"));
	}
}

void UMainMenu::MenuTeardown()
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
