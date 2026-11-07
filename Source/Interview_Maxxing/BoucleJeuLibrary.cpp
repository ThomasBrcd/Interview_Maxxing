#include "BoucleJeuLibrary.h"
#include "Components/PanelWidget.h"
#include "Engine/GameInstance.h"
#include "UObject/UnrealType.h"
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

void UBoucleJeuLibrary::RetirerOffresPostulees(UObject* WorldContextObject, UPanelWidget* ListeMails)
{
	const UWorld* Monde = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const UGameInstance* GI = Monde ? Monde->GetGameInstance() : nullptr;
	if (!GI || !ListeMails)
	{
		return;
	}

	const FArrayProperty* PropOffres = FindFProperty<FArrayProperty>(GI->GetClass(), TEXT("OffresPostulees"));
	if (!PropOffres || !CastField<FNameProperty>(PropOffres->Inner))
	{
		return;
	}
	TSet<FName> Postulees;
	FScriptArrayHelper Tableau(PropOffres, PropOffres->ContainerPtrToValuePtr<void>(GI));
	for (int32 Index = 0; Index < Tableau.Num(); ++Index)
	{
		Postulees.Add(*reinterpret_cast<const FName*>(Tableau.GetRawPtr(Index)));
	}

	for (UWidget* Element : ListeMails->GetAllChildren())
	{
		const FNameProperty* PropId = Element ? FindFProperty<FNameProperty>(Element->GetClass(), TEXT("ID_Mail")) : nullptr;
		if (PropId && Postulees.Contains(PropId->GetPropertyValue_InContainer(Element)))
		{
			Element->RemoveFromParent();
		}
	}
}
