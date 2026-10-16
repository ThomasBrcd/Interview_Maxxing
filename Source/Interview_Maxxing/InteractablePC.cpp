#include "InteractablePC.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/WidgetComponent.h"

// Sets default values
AInteractablePC::AInteractablePC()
{
	PrimaryActorTick.bCanEverTick = false;
    bIsPlayerNear = false;

	PCMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PCMesh"));
	RootComponent = PCMesh;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AInteractablePC::OnOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AInteractablePC::OnOverlapEnd);

    PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
    PromptWidget->SetupAttachment(RootComponent);
    PromptWidget->SetVisibility(false);
    PromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
    PromptWidget->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
}

void AInteractablePC::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this && OtherActor->IsA(APawn::StaticClass()))
    {
        bIsPlayerNear = true;
    }
}

void AInteractablePC::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor != this && OtherActor->IsA(APawn::StaticClass()))
    {
        bIsPlayerNear = false;
    }
}

void AInteractablePC::InteractWithPC()
{
    if (bIsPlayerNear)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            // 1. Transition fluide vers la caméra du PC (durée 0.5s)
            PC->SetViewTargetWithBlend(this, 0.5f);

            // 2. Afficher la souris et autoriser les clics
            PC->bShowMouseCursor = true;
            FInputModeGameAndUI InputMode;
            InputMode.SetHideCursorDuringCapture(false);
            PC->SetInputMode(InputMode);

            // NOUVEAU : Bloquer les mouvements et la rotation de la caméra
            PC->SetIgnoreMoveInput(true);
            PC->SetIgnoreLookInput(true);
        }

        // Masquer les aides visuelles
        if (PromptWidget) PromptWidget->SetVisibility(false);
        if (PCMesh) PCMesh->SetRenderCustomDepth(false);

        // 3. Appeler l'événement Blueprint pour charger les boutons/CV
        OnPCInteracted();
    }
}