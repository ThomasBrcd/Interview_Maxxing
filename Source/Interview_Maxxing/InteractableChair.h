#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableChair.generated.h"

UCLASS()
class INTERVIEW_MAXXING_API AInteractableChair : public AActor
{
    GENERATED_BODY()
    
public:    
    AInteractableChair();

    // Fonction à appeler depuis le Blueprint du joueur quand il appuie sur 'E'
    UFUNCTION(BlueprintCallable, Category="Interaction")
    void InteractWithChair();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* ChairMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* TriggerBox;

    // Composant UI pour afficher le bouton "E"
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UWidgetComponent* PromptWidget;
    
    bool bIsPlayerNear;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Événement Blueprint pour déclencher l'animation de la chaise ou lancer l'entretien
    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void StartChairInteraction();
};