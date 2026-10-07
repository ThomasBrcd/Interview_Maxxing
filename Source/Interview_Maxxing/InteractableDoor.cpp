#include "InteractableDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Pawn.h"

AInteractableDoor::AInteractableDoor()
{
    PrimaryActorTick.bCanEverTick = false;
    bIsPlayerNear = false;

    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    RootComponent = DoorMesh;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(RootComponent);
    TriggerBox->SetBoxExtent(FVector(100.f, 50.f, 150.f));

    PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
    PromptWidget->SetupAttachment(RootComponent);
    PromptWidget->SetVisibility(false);
    PromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
    PromptWidget->SetRelativeLocation(FVector(0.f, 0.f, 100.f));

    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AInteractableDoor::OnOverlapBegin);
    TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AInteractableDoor::OnOverlapEnd);
}

void AInteractableDoor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this && OtherActor->IsA(APawn::StaticClass()))
    {
        bIsPlayerNear = true;
        if (PromptWidget) PromptWidget->SetVisibility(true);
        if (DoorMesh) DoorMesh->SetRenderCustomDepth(true);
    }
}

void AInteractableDoor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor != this && OtherActor->IsA(APawn::StaticClass()))
    {
        bIsPlayerNear = false;
        if (PromptWidget) PromptWidget->SetVisibility(false);
        if (DoorMesh) DoorMesh->SetRenderCustomDepth(false);
    }
}

void AInteractableDoor::InteractWithDoor()
{
    if (bIsPlayerNear)
    {
        if (PromptWidget) PromptWidget->SetVisibility(false);
        if (DoorMesh) DoorMesh->SetRenderCustomDepth(false);
        ShowQuitDialogue();
    }
}