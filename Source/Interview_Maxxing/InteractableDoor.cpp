#include "InteractableDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"

AInteractableDoor::AInteractableDoor()
{
    PrimaryActorTick.bCanEverTick = false;

    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    RootComponent = DoorMesh;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(RootComponent);

    // On fait une boîte plus haute pour une porte
    TriggerBox->SetBoxExtent(FVector(100.f, 50.f, 150.f));

    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AInteractableDoor::OnOverlapBegin);
}

void AInteractableDoor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Si quelque chose traverse la porte, et que c'est le Joueur
    if (OtherActor && OtherActor != this)
    {
        APawn* PlayerPawn = Cast<APawn>(OtherActor);
        if (PlayerPawn)
        {
            // On déclenche l'événement Blueprint pour afficher le menu !
            ShowQuitDialogue();
        }
    }
}