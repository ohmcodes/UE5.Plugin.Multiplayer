// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONSUE58_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 InNumPublicConnections = 4, FString InMatchType = TEXT("FreeForAll"), FString InPathToLobby = TEXT("/MultiplayerSessionsUE58/Levels/Lobby"));
	

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	// Callbacks for the custom delegates on the MultiplayerSessionsSubsystem
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
	
private:

	UPROPERTY(meta = (BindWidget))
	class UButton* btn_create;
	UPROPERTY(meta = (BindWidget))
	class UButton* btn_join;

	UFUNCTION()
	void OnCreateClicked();
	UFUNCTION()
	void OnJoinClicked();

	void MenuTearDown();

	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	int32 NumPublicConnections{ 4 };
	FString MatchType{ TEXT("FreeForAll") };
	FString PathToLobby{ TEXT("/MultiplayerSessionsUE58/Levels/Lobby") };
};
