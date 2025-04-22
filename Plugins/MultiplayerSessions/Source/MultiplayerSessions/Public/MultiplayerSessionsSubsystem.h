// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"

/// 
/// Declaring our own custom delegates for the Menu class to bind callbacks to
/// 

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayer_OnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayer_OnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful); // Not a UClass therefore cannot use in blueprints
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayer_OnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayer_OnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayer_OnStartSessionComplete, bool, bWasSuccessful);


UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UMultiplayerSessionsSubsystem();
	
	//
	// To handle session functionality. The Menu class will call these
	//

	void CreateSession(int32 NumPublicConnections, FString MatchType);
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();

	//
	// Our own custom delegates for the Menu class to bind callbacks to
	//

	FMultiplayer_OnCreateSessionComplete		Multiplayer_OnCreateSessionComplete;
	FMultiplayer_OnFindSessionsComplete			Multiplayer_OnFindSessionsComplete;
	FMultiplayer_OnJoinSessionComplete			Multiplayer_OnJoinSessionComplete;
	FMultiplayer_OnDestroySessionComplete		Multiplayer_OnDestroySessionComplete;
	FMultiplayer_OnStartSessionComplete			Multiplayer_OnStartSessionComplete;

protected:

	//
	// Internal callbacks for the delegates we'll add to the Online Session Interface delegate list.
	// These don't need to be called outside this class.
	//

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
	
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	//
	// To add to the Online Session Interface delegate list.
	// We'll bind our MultiplayerSessionsSubsystem internal callbacks to these.
	// Delegate handles (one for each delegate) are used to remove delegates from delegate list once no longer needed
	//

	FOnCreateSessionCompleteDelegate			CreateSessionCompleteDelegate;
	FDelegateHandle								CreateSessionCompleteDelegate_Handle;
	FOnFindSessionsCompleteDelegate				FindSessionsCompleteDelegate;
	FDelegateHandle								FindSessionsCompleteDelegate_Handle;
	FOnJoinSessionCompleteDelegate				JoinSessionCompleteDelegate;
	FDelegateHandle								JoinSessionCompleteDelegate_Handle;
	FOnDestroySessionCompleteDelegate			DestroySessionCompleteDelegate;
	FDelegateHandle								DestroySessionCompleteDelegate_Handle;
	FOnStartSessionCompleteDelegate				StartSessionCompleteDelegate;
	FDelegateHandle								StartSessionCompleteDelegate_Handle;


	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;

};
