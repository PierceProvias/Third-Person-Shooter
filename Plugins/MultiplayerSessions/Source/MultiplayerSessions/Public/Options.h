// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Options.generated.h"

class UButton;
class UWidgetSwitcher;
class UComboBoxString;
class UCheckBox;

UCLASS()
class MULTIPLAYERSESSIONS_API UOptions : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void MenuSetup();

	virtual void NativeConstruct() override;
	

protected:

	void InitializeResolutionComboBox();
	void InitializeVSync();
	
	UPROPERTY()
	TObjectPtr<UGameUserSettings> GameUserSettings;

	/*
	* Resolution Settings
	*/
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UComboBoxString> ResolutionComboBox;

	UPROPERTY()
	TArray<FIntPoint> Resolutions;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UCheckBox> VSyncCheckBox;

	virtual bool Initialize() override;
	virtual void NativeDestruct() override;


	UFUNCTION()
	void OnResolutionChanged(FString InSelectedItem, ESelectInfo::Type InSelectionType);

	UFUNCTION()
	void OnVSyncChanged(bool InIsChecked);

private:

	/*
	* Widget Switcher Option Categories 
	*/
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> GameplayButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ControllerButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MouseKeyboardButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AccessibilityButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AudioButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> VideoButton;


	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> WindowLeftButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> WindowRightButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton;

	UFUNCTION()
	void GameplayButtonClicked();

	UFUNCTION()
	void ControllerButtonClicked();

	UFUNCTION()
	void MouseKeyboardButtonClicked();

	UFUNCTION()
	void AccessibilityButtonClicked();

	UFUNCTION()
	void AudioButtonClicked();

	UFUNCTION()
	void VideoButtonClicked();

	UFUNCTION()
	void WindowLeftButtonClicked();

	UFUNCTION()
	void WindowRightButtonClicked();

	UFUNCTION()
	void ResolutionLeftButtonClicked();

	UFUNCTION()
	void ResolutionRightButtonClicked();

	UFUNCTION()
	void GraphicsLeftButtonClicked();

	UFUNCTION()
	void GraphicsRightButtonClicked();

	UFUNCTION()
	void VSyncLeftButtonClicked();

	UFUNCTION()
	void VSyncRightButtonClicked();

	UFUNCTION()
	void BackButtonClicked();

	void MenuTeardown();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;

	int32 SwitcherIndex = 0;

	void UpdateSwitcherPanel(int32 Index);
	// TODO: Add WidgetSwitcher for options menu and character selection

	int32 WindowMode = EWindowMode::Fullscreen;
};
