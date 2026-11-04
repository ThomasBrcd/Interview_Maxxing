#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UCRTInteractionComponent.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;
class UWidgetInteractionComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INTERVIEW_MAXXING_API UCRTInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:	
    UCRTInteractionComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Initialise les composants de cette télé (à appeler au BeginPlay du Blueprint de la télé) */
    UFUNCTION(BlueprintCallable, Category = "CRT Interaction")
    void SetupCRTComponents(UStaticMeshComponent* InScreenMesh, UWidgetComponent* InWidgetComponent);

    /** Active ou désactive l'interaction avec cet écran (ex: quand le joueur s'assoit devant le PC) */
    UFUNCTION(BlueprintCallable, Category = "CRT Interaction")
    void SetInteractionActive(bool bActive, UWidgetInteractionComponent* InWidgetInteraction = nullptr);

    /** Canal UV utilisé par le Material de l'écran sur le Mesh 3D (généralement 0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT Settings")
    int32 UVChannel = 0;

    /** Distance maximale du rayon depuis la caméra */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT Settings")
    float TraceDistance = 300.0f;

    /** Canal de collision utilisé pour détecter la vitre */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT Settings")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    /** Affiche un point rouge là où le rayon touche la vitre et un point vert sur le plan virtuel du Widget */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT Settings")
    bool bDrawDebug = false;

private:
    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> ScreenMesh;

    UPROPERTY()
    TObjectPtr<UWidgetComponent> SourceWidgetComponent;

    UPROPERTY()
    TObjectPtr<UWidgetInteractionComponent> ActiveWidgetInteraction;
};