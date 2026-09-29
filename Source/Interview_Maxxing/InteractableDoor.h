#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableDoor.generated.h"

UCLASS()
class INTERVIEW_MAXXING_API AInteractableDoor : public AActor
{
    GENERATED_BODY()

public:
    AInteractableDoor();

protected:
    // Le modèle visuel de la porte (le plan)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* DoorMesh;

    // La zone de détection
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* TriggerBox;

    // Fonction de collision
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // Ordre envoyé au Blueprint pour afficher le menu UI
    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void ShowQuitDialogue();
};