// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenu.generated.h"

class UButton;

UCLASS()
class MULTIPLAYERSESSIONS_API UPauseMenu : public UUserWidget
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
	TObjectPtr<UButton> ResumeButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> OptionsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;

	UFUNCTION()
	void ResumeButtonPressed();

	UFUNCTION()
	void OptionsButtonPressed();

	UFUNCTION()
	void QuitButtonPressed();

	void MenuTeardown();
};
