#include "BoucleJeuLibrary.h"
#include "InteractablePC.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

void UBoucleJeuLibrary::RendreControleAuJoueur(UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	// Arrête la lecture du curseur sur les écrans CRT
	for (TActorIterator<AInteractablePC> It(World); It; ++It)
	{
		It->ExitPC();
	}

	if (APawn* Pawn = PC->GetPawn())
	{
		PC->SetViewTargetWithBlend(Pawn, 0.5f);
	}
	PC->SetInputMode(FInputModeGameOnly());
	PC->bShowMouseCursor = false;
	PC->ResetIgnoreMoveInput();
	PC->ResetIgnoreLookInput();
}
