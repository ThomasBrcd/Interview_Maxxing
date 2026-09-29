// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TimerHandle.h"
#include "InteractableBed.generated.h"

UCLASS()
class INTERVIEW_MAXXING_API AInteractableBed : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractableBed();

protected:
    // Le modèle 3D du lit
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* BedMesh;

    // La zone pour détecter le joueur
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* TriggerBox;

    // Fonctions appelées quand le joueur entre ou sort de la zone
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void GoToSleep();

    // La fonction appelée après 3 secondes
    void WakeUp();

private:
    bool bIsPlayerNear;
    class APawn* PlayerPawn;
    FTimerHandle SleepTimerHandle;
};

