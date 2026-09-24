// Fill out your copyright notice in the Description page of Project Settings.


#include "GM_Lobby.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void AGM_Lobby::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GameState)
	{
		int32 NumberOfPlayers = GameState->PlayerArray.Num();

		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Green, FString::Printf(TEXT("Number of Players: %d"), NumberOfPlayers));

			APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
			if (PlayerState)
			{
				FString PlayerName = PlayerState->GetPlayerName();

				GEngine->AddOnScreenDebugMessage(2, 60.f, FColor::Green, FString::Printf(TEXT("Player %s has joined the lobby."), *PlayerName));
			}
		}
	}
}

void AGM_Lobby::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	APlayerState* PlayerState = Exiting->GetPlayerState<APlayerState>();
	if (PlayerState)
	{
		if(GEngine)
		{
			int32 NumberOfPlayers = GameState->PlayerArray.Num();
			GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Green, FString::Printf(TEXT("Number of Players: %d"), NumberOfPlayers - 1));

			FString PlayerName = PlayerState->GetPlayerName();

			GEngine->AddOnScreenDebugMessage(2, 60.f, FColor::Green, FString::Printf(TEXT("Player %s has left the lobby."), *PlayerName));
		}
	}
}
