#include "InteractablePC.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
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
        if (PromptWidget)
        {
            PromptWidget->SetVisibility(true);
        }
        if (PCMesh)
        {
            PCMesh->SetRenderCustomDepth(true);
        }
    }
}

void AInteractablePC::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor != this && OtherActor->IsA(APawn::StaticClass()))
    {
        bIsPlayerNear = false;
    }
    if (PromptWidget) PromptWidget->SetVisibility(false);
    if (PCMesh) PCMesh->SetRenderCustomDepth(false);
}

void AInteractablePC::InteractWithPC()
{
    if (bIsPlayerNear)
    {
        ShowPCInterface();
        if (PromptWidget) PromptWidget->SetVisibility(false);
        if (PCMesh) PCMesh->SetRenderCustomDepth(false);
    }
}