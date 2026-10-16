#include "InteractablePC.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/WidgetComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "InputCoreTypes.h"
#include "DrawDebugHelpers.h"

// Sets default values
AInteractablePC::AInteractablePC()
{
	// Le Tick est autorisé mais désactivé au lancement pour ne rien consommer hors interaction
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
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

	// Configuration du WidgetInteractionComponent en mode Custom (piloté par nos UVs)
	CRTWidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("CRTWidgetInteraction"));
	CRTWidgetInteraction->SetupAttachment(RootComponent);
	CRTWidgetInteraction->InteractionSource = EWidgetInteractionSource::Custom;
	CRTWidgetInteraction->bEnableHitTesting = true;
}

void AInteractablePC::SetupCRTComponents(UStaticMeshComponent* InScreenMesh, UWidgetComponent* InWidgetComponent)
{
    if (InScreenMesh) CRTScreenMesh = InScreenMesh;
    if (InWidgetComponent) CRTWidgetComp = InWidgetComponent;

    // SÉCURITÉ AUTOMATIQUE : Si le Blueprint ne les a pas fournis, on les cherche nous-mêmes par leur nom !
    if (!CRTScreenMesh)
    {
        // Cherche un composant nommé "PCScreen" dans ton Blueprint
        TArray<UStaticMeshComponent*> MeshComps;
        GetComponents<UStaticMeshComponent>(MeshComps);
        for (UStaticMeshComponent* MeshComp : MeshComps)
        {
            if (MeshComp && MeshComp->GetName().Contains(TEXT("PCScreen")))
            {
                CRTScreenMesh = MeshComp;
                break;
            }
        }
    }

    if (!CRTWidgetComp)
    {
        // Cherche le WidgetComponent nommé "PCUI" ou prend le premier trouvé
        TArray<UWidgetComponent*> WidgetComps;
        GetComponents<UWidgetComponent>(WidgetComps);
        for (UWidgetComponent* WComp : WidgetComps)
        {
            if (WComp && WComp != PromptWidget) // Évite de prendre le PromptWidget de la touche E
            {
                CRTWidgetComp = WComp;
                break;
            }
        }
    }

    // Instanciation de secours pour le WidgetInteraction
    if (!CRTWidgetInteraction)
    {
        CRTWidgetInteraction = FindComponentByClass<UWidgetInteractionComponent>();
        if (!CRTWidgetInteraction)
        {
            CRTWidgetInteraction = NewObject<UWidgetInteractionComponent>(this, TEXT("CRTWidgetInteraction_Dynamic"));
            CRTWidgetInteraction->RegisterComponent();
            CRTWidgetInteraction->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        }
    }

    if (CRTWidgetInteraction)
    {
        CRTWidgetInteraction->InteractionSource = EWidgetInteractionSource::Custom;
        CRTWidgetInteraction->bEnableHitTesting = true;
		CRTWidgetInteraction->VirtualUserIndex = 1;
		CRTWidgetInteraction->PointerIndex = 1;
    }

    if (CRTWidgetComp)
    {
        CRTWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CRTWidgetComp->SetWindowFocusable(true);
    }

    if (CRTScreenMesh)
    {
        CRTScreenMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        CRTScreenMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    }
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
			// Transition fluide vers la caméra du PC (durée 0.5s)
			PC->SetViewTargetWithBlend(this, 0.5f);

			// Afficher la souris et autoriser les clics sans bloquer les inputs
			PC->bShowMouseCursor = true;
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);

			// Bloquer les mouvements et la rotation de la caméra
			PC->SetIgnoreMoveInput(true);
			PC->SetIgnoreLookInput(true);
		}

		// Masquer les aides visuelles
		if (PromptWidget) PromptWidget->SetVisibility(false);
		if (PCMesh) PCMesh->SetRenderCustomDepth(false);

		// Activer la lecture des UVs à chaque frame
		SetActorTickEnabled(true);

		// 3. Appeler l'événement Blueprint pour charger les boutons/CV
		OnPCInteracted();
	}
}

void AInteractablePC::ExitPC()
{
	SetActorTickEnabled(false);
	bIsLeftMousePressed = false;

	if (CRTWidgetInteraction)
	{
		CRTWidgetInteraction->SetCustomHitResult(FHitResult());
	}
}

void AInteractablePC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Si les composants sont nuls, on tente de les auto-initialiser à la volée
    if (!CRTScreenMesh || !CRTWidgetComp || !CRTWidgetInteraction)
    {
        SetupCRTComponents(nullptr, nullptr);
    }

    // Si malgré l'auto-détection c'est toujours nul, on prévient en rouge
    if (!CRTScreenMesh || !CRTWidgetComp || !CRTWidgetInteraction)
    {
        FString Missing = TEXT("");
        if (!CRTScreenMesh) Missing += TEXT("[PCScreen introuvable] ");
        if (!CRTWidgetComp) Missing += TEXT("[PCUI introuvable] ");
        if (!CRTWidgetInteraction) Missing += TEXT("[WidgetInteraction manquant] ");

        if (GEngine) GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Red, FString::Printf(TEXT("ERREUR CRT : %s"), *Missing));
        return;
    }

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	// Test 2 : Vérifier la déprojection de la souris
	FVector WorldOrigin, WorldDirection;
	if (!PC->DeprojectMousePositionToWorld(WorldOrigin, WorldDirection))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Orange, TEXT("ERREUR CRT : Impossible de déprojeter la souris (Souris hors de la fenêtre ?)."));
		return;
	}

	FVector TraceStart = WorldOrigin;
	FVector TraceEnd = TraceStart + (WorldDirection * 1000.0f); // Augmenté à 1000 au cas où la caméra est loin

	// Paramètres du Line Trace
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CRTTrace), true);
	QueryParams.bReturnFaceIndex = true;

	// IMPORTANT : On ignore le boîtier du PC, la TriggerBox et les Widgets pour être sûr de toucher PCScreen !
	QueryParams.AddIgnoredComponent(PCMesh);
	QueryParams.AddIgnoredComponent(TriggerBox);
	QueryParams.AddIgnoredComponent(PromptWidget);
	QueryParams.AddIgnoredComponent(CRTWidgetComp);

	FHitResult MeshHit;
	bool bHit = GetWorld()->LineTraceSingleByChannel(MeshHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	if (!bHit)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Yellow, TEXT("CRT TRACE : Le rayon ne touche RIEN (Vérifie la collision de PCScreen)."));
		CRTWidgetInteraction->SetCustomHitResult(FHitResult());
		return;
	}

	// Test 3 : Vérifier si on touche bien PCScreen ou un autre objet
	if (MeshHit.GetComponent() != CRTScreenMesh)
	{
		FString HitName = MeshHit.GetComponent() ? MeshHit.GetComponent()->GetName() : TEXT("Inconnu");
		if (GEngine) GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Yellow, FString::Printf(TEXT("CRT TRACE : Le rayon est bloqué par '%s' au lieu de PCScreen !"), *HitName));
		CRTWidgetInteraction->SetCustomHitResult(FHitResult());
		return;
	}

	// On dessine le point ROUGE dès qu'on touche PCScreen (même avant de lire les UVs)
	if (bDrawDebugCRT)
	{
		DrawDebugSphere(GetWorld(), MeshHit.ImpactPoint, 1.5f, 8, FColor::Red, false, -1.0f, SDPG_Foreground);
	}

	// Test 4 : Vérifier l'extraction des UVs
	FVector2D UV;
	if (UGameplayStatics::FindCollisionUV(MeshHit, UVChannel, UV))
	{
		if (bFlipU) UV.X = 1.0f - UV.X;
		if (bFlipV) UV.Y = 1.0f - UV.Y;

		if (GEngine) GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Green, FString::Printf(TEXT("CRT OK ! UV = (%.2f, %.2f)"), UV.X, UV.Y));

		FVector2D DrawSize = CRTWidgetComp->GetDrawSize();
		FVector2D Pivot = CRTWidgetComp->GetPivot();

		FVector LocalHitPos(
			0.0f,
			(UV.X - Pivot.X) * DrawSize.X,
			(Pivot.Y - UV.Y) * DrawSize.Y
		);

		FVector VirtualWorldImpact = CRTWidgetComp->GetComponentTransform().TransformPosition(LocalHitPos);
		FVector WidgetNormal = CRTWidgetComp->GetForwardVector();

		if (bDrawDebugCRT)
		{
			DrawDebugSphere(GetWorld(), VirtualWorldImpact, 1.5f, 8, FColor::Green, false, -1.0f, SDPG_Foreground);
		}

		FHitResult CustomHit(this, CRTWidgetComp, VirtualWorldImpact, WidgetNormal);
		CustomHit.bBlockingHit = true;
		CustomHit.Location = VirtualWorldImpact;
		CustomHit.ImpactPoint = VirtualWorldImpact;
		CustomHit.ImpactNormal = WidgetNormal;
		CustomHit.Distance = MeshHit.Distance;

		LastValidCustomHit = CustomHit;
		CRTWidgetInteraction->SetCustomHitResult(CustomHit);

		bool bCurrentlyDown = PC->IsInputKeyDown(EKeys::LeftMouseButton);

		if (bCurrentlyDown && !bIsLeftMousePressed)
		{
			bIsLeftMousePressed = true;
			CRTWidgetInteraction->PressPointerKey(EKeys::LeftMouseButton);
		}
		else if (!bCurrentlyDown && bIsLeftMousePressed)
		{
			bIsLeftMousePressed = false;
			CRTWidgetInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
		}

		float WheelDelta = PC->GetInputAnalogKeyState(EKeys::MouseWheelAxis);
		if (FMath::Abs(WheelDelta) > KINDA_SMALL_NUMBER)
		{
			CRTWidgetInteraction->ScrollWheel(WheelDelta);
		}

		return;
	}
	if (bIsLeftMousePressed)
	{
		CRTWidgetInteraction->SetCustomHitResult(LastValidCustomHit);

		if (!PC->IsInputKeyDown(EKeys::LeftMouseButton))
		{
			bIsLeftMousePressed = false;
			CRTWidgetInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
		}
		return;
	}

	CRTWidgetInteraction->SetCustomHitResult(FHitResult());
}

void AInteractablePC::OnCRTLeftClickPressed()
{
	if (IsActorTickEnabled() && CRTWidgetInteraction)
	{
		CRTWidgetInteraction->PressPointerKey(EKeys::LeftMouseButton);
	}
}

void AInteractablePC::OnCRTLeftClickReleased()
{
	if (IsActorTickEnabled() && CRTWidgetInteraction)
	{
		CRTWidgetInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
	}
}

void AInteractablePC::OnCRTScrollWheel(float ScrollDelta)
{
	if (IsActorTickEnabled() && CRTWidgetInteraction)
	{
		CRTWidgetInteraction->ScrollWheel(ScrollDelta);
	}
}