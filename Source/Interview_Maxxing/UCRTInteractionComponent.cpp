#include "UCRTInteractionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"

UCRTInteractionComponent::UCRTInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    // Désactivé par défaut pour ne rien coûter en performance tant qu'on n'utilise pas le PC
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCRTInteractionComponent::SetupCRTComponents(UStaticMeshComponent* InScreenMesh, UWidgetComponent* InWidgetComponent)
{
    ScreenMesh = InScreenMesh;
    SourceWidgetComponent = InWidgetComponent;

    if (SourceWidgetComponent)
    {
        // On coupe les collisions du WidgetComponent pour qu'il ne bloque jamais le rayon devant la vitre
        SourceWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (ScreenMesh)
    {
        // On s'assure que le Mesh de la vitre répond bien au canal de Trace choisi
        ScreenMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        ScreenMesh->SetCollisionResponseToChannel(TraceChannel, ECR_Block);
    }
}

void UCRTInteractionComponent::SetInteractionActive(bool bActive, UWidgetInteractionComponent* InWidgetInteraction)
{
    ActiveWidgetInteraction = bActive ? InWidgetInteraction : nullptr;

    if (ActiveWidgetInteraction)
    {
        // On force le WidgetInteractionComponent en mode Custom pour qu'il écoute nos coordonnées UV
        ActiveWidgetInteraction->InteractionSource = EWidgetInteractionSource::Custom;
    }
    else if (InWidgetInteraction)
    {
        // On nettoie le HitResult en quittant l'écran pour relâcher les états "Hovered" des boutons
        InWidgetInteraction->SetCustomHitResult(FHitResult());
    }

    SetComponentTickEnabled(bActive);
}

void UCRTInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!ScreenMesh || !SourceWidgetComponent || !ActiveWidgetInteraction) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC) return;

    // 1. Déprojection de la position 2D du curseur Windows vers un rayon 3D depuis la caméra
    FVector WorldOrigin, WorldDirection;
    if (!PC->DeprojectMousePositionToWorld(WorldOrigin, WorldDirection)) return;

    FVector TraceStart = WorldOrigin;
    FVector TraceEnd = TraceStart + (WorldDirection * TraceDistance);

    // 2. Paramètres du Line Trace Complexe (par triangle)
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CRTInteractionTrace), true); // true = Trace Complex
    QueryParams.bReturnFaceIndex = true; // Indispensable pour récupérer les UVs du triangle touché !

    FHitResult MeshHit;
    bool bHit = GetWorld()->LineTraceSingleByChannel(MeshHit, TraceStart, TraceEnd, TraceChannel, QueryParams);

    // 3. Vérification qu'on a bien touché le Mesh de la vitre de CETTE télé
    if (bHit && MeshHit.GetComponent() == ScreenMesh)
    {
        FVector2D UV;
        if (UGameplayStatics::FindCollisionUV(MeshHit, UVChannel, UV))
        {
            if (bDrawDebug)
            {
                // Point ROUGE : Impact réel sur la vitre bombée
                DrawDebugPoint(GetWorld(), MeshHit.ImpactPoint, 8.0f, FColor::Red, false, -1.0f, SDPG_Foreground);
            }

            // 4. Conversion UV (0..1) -> Coordonnées Locales 3D du WidgetComponent
            FVector2D DrawSize = SourceWidgetComponent->GetDrawSize();
            FVector2D Pivot = SourceWidgetComponent->GetPivot();

            // Dans l'espace local d'un WidgetComponent Unreal :
            // X = Profondeur (0), Y = Axe Horizontal (Gauche -> Droite), Z = Axe Vertical (Bas -> Haut)
            FVector LocalHitPos(
                0.0f,
                (UV.X - Pivot.X) * DrawSize.X,
                (Pivot.Y - UV.Y) * DrawSize.Y
            );

            // Conversion en World Space par rapport à la transformation actuelle du WidgetComponent
            FVector VirtualWorldImpact = SourceWidgetComponent->GetComponentTransform().TransformPosition(LocalHitPos);
            FVector WidgetNormal = SourceWidgetComponent->GetForwardVector();

            if (bDrawDebug)
            {
                // Point VERT : Impact virtuel re-projeté sur le WidgetComponent plat
                DrawDebugPoint(GetWorld(), VirtualWorldImpact, 8.0f, FColor::Green, false, -1.0f, SDPG_Foreground);
            }

            // 5. Fabrication et envoi du Custom Hit Result au WidgetInteractionComponent
            FHitResult CustomWidgetHit(SourceWidgetComponent->GetOwner(), SourceWidgetComponent, VirtualWorldImpact, WidgetNormal);
            CustomWidgetHit.bBlockingHit = true;
            CustomWidgetHit.Location = VirtualWorldImpact;
            CustomWidgetHit.ImpactPoint = VirtualWorldImpact;
            CustomWidgetHit.ImpactNormal = WidgetNormal;
            CustomWidgetHit.Distance = MeshHit.Distance;

            ActiveWidgetInteraction->SetCustomHitResult(CustomWidgetHit);
            return;
        }
    }

    // Si la souris sort de l'écran, on envoie un HitResult vide
    ActiveWidgetInteraction->SetCustomHitResult(FHitResult());
}