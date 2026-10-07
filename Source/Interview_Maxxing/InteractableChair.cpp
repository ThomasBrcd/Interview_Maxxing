#include "InteractableChair.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h" // Nécessaire pour le composant d'interface
#include "GameFramework/Pawn.h"

AInteractableChair::AInteractableChair()
{
    PrimaryActorTick.bCanEverTick = false;
    bIsPlayerNear = false;

    ChairMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChairMesh"));
    RootComponent = ChairMesh;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(RootComponent);
    TriggerBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));

    // Configuration du widget pour afficher la touche "E"
    PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
    PromptWidget->SetupAttachment(RootComponent);
    PromptWidget->SetVisibility(false); // Caché par défaut
    PromptWidget->SetWidgetSpace(EWidgetSpace::Screen); // S'affiche face à l'écran, pas dans la 3D pure
    PromptWidget->SetRelativeLocation(FVector(0.f, 0.f, 100.f)); // Un peu au-dessus de la chaise

    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AInteractableChair::OnOverlapBegin);
    TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AInteractableChair::OnOverlapEnd);
}

void AInteractableChair::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this && OtherActor->IsA(APawn::StaticClass()))
    {
        bIsPlayerNear = true;

        // On vérifie que le pointeur n'est pas vide avant de l'utiliser
        if (PromptWidget)
        {
            PromptWidget->SetVisibility(true);
        }
        
        if (ChairMesh)
        {
            ChairMesh->SetRenderCustomDepth(true);
        }
    }
}

void AInteractableChair::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor != this && OtherActor->IsA(APawn::StaticClass()))
    {
        bIsPlayerNear = false;

        if (PromptWidget)
        {
            PromptWidget->SetVisibility(false);
        }

        if (ChairMesh)
        {
            ChairMesh->SetRenderCustomDepth(false);
        }
    }
}

void AInteractableChair::InteractWithChair()
{
    if (bIsPlayerNear)
    {
        if (PromptWidget)
        {
            PromptWidget->SetVisibility(false);
        }
        
        if (ChairMesh)
        {
            ChairMesh->SetRenderCustomDepth(false);
        }
        
        StartChairInteraction();
    }
}