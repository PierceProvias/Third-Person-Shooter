// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Blueprint/UserWidget.h"
#include "SelectionOption.h"
#include "SelectionBase.generated.h"

class UTextBlock;

UCLASS(Blueprintable, BlueprintType)
class MULTIPLAYERSESSIONS_API USelectionBase : public UUserWidget
{
	GENERATED_BODY()
	
public:

	USelectionBase(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	void Clear();
	void AddOption(const FSelectionOption& InOption);
	void SetCurrentSelection(int InIndex);

	UFUNCTION(BlueprintCallable)
	void SelectPrevious();

	UFUNCTION(BlueprintCallable)
	void SelectNext();

	DECLARE_DELEGATE_OneParam(FOnSelectionChange, int);
	FOnSelectionChange OnSelectionChange;

protected:

	UFUNCTION()
	UWidget* OnNavigation(EUINavigation InNavigation);

	void UpdateCurrentSelection();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Options")
	TArray<FSelectionOption> Options;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Label;

	int CurrentSelection;

private:

};
