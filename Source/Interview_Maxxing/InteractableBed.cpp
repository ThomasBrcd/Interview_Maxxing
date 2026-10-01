#include "InteractableBed.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "Components/WidgetComponent.h"

// Constructeur
AInteractableBed::AInteractableBed()
{
    PrimaryActorTick.bCanEverTick = false;

    BedMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BedMesh"));
    RootComponent = BedMesh;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(RootComponent);
    TriggerBox->SetBoxExtent(FVector(100.f, 100.f, 50.f));

    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AInteractableBed::OnOverlapBegin);
    TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AInteractableBed::OnOverlapEnd);

    PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
    PromptWidget->SetupAttachment(RootComponent);
    PromptWidget->SetVisibility(false);
    PromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
    PromptWidget->SetRelativeLocation(FVector(0.f, 0.f, 100.f));

    bIsPlayerNear = false;
}

void AInteractableBed::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this)
    {
        PlayerPawn = Cast<APawn>(OtherActor);
        if (PlayerPawn) bIsPlayerNear = true;
    }
    if (PromptWidget)
        {
            PromptWidget->SetVisibility(true);
        }
        if (BedMesh)
        {
            BedMesh->SetRenderCustomDepth(true);
        }
}

void AInteractableBed::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor == PlayerPawn)
    {
        bIsPlayerNear = false;
        PlayerPawn = nullptr;
    }
    if (PromptWidget) PromptWidget->SetVisibility(false);
    if (BedMesh) BedMesh->SetRenderCustomDepth(false);
}

void AInteractableBed::GoToSleep()
{
    if (!bIsPlayerNear || !PlayerPawn) return;

    APlayerController* PC = Cast<APlayerController>(PlayerPawn->GetController());
    if (PC && PC->PlayerCameraManager)
    {
        PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, 1.0f, FLinearColor::Black, false, true);
        PlayerPawn->DisableInput(PC);
        GetWorld()->GetTimerManager().SetTimer(SleepTimerHandle, this, &AInteractableBed::WakeUp, 3.0f, false);

        if (PromptWidget) PromptWidget->SetVisibility(false);
        if (BedMesh) BedMesh->SetRenderCustomDepth(false);
    }
}

void AInteractableBed::WakeUp()
{
    APlayerController* PC = Cast<APlayerController>(PlayerPawn->GetController());
    if (PC && PC->PlayerCameraManager)
    {
        PC->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, 1.0f, FLinearColor::Black, false, true);
        PlayerPawn->EnableInput(PC);

        FVector WakeUpLocation = GetActorLocation() + FVector(150.f, 0.f, 50.f);
        PlayerPawn->SetActorLocation(WakeUpLocation);
    }
}