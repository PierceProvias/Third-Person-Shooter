// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"


class UButton;
class UOptions;

UCLASS()
class MULTIPLAYERSESSIONS_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()


public:

	UFUNCTION(BlueprintCallable)
	void MenuSetup();

protected:

	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PlayButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CharacterButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> OptionsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;

	UFUNCTION()
	void PlayButtonPressed();

	UFUNCTION()
	void CharacterButtonPressed();

	UFUNCTION()
	void OptionsButtonPressed();

	UFUNCTION()
	void QuitButtonPressed();

	void MenuTeardown();

	TObjectPtr<UOptions> OptionsMenu;

	


};
