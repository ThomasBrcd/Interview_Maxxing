#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractablePC.generated.h"

UCLASS()
class INTERVIEW_MAXXING_API AInteractablePC : public AActor
{
	GENERATED_BODY()
	
public:	
	AInteractablePC();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void InteractWithPC();

	/** À appeler quand le joueur quitte le PC pour couper le Tick et nettoyer le survol */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ExitPC();

	/** À appeler dans le BeginPlay du Blueprint en branchant PCScreen et PCUI */
	UFUNCTION(BlueprintCallable, Category = "CRT Interaction")
	void SetupCRTComponents(class UStaticMeshComponent* InScreenMesh, class UWidgetComponent* InWidgetComponent);

	/** Fonctions à relier à tes inputs de souris (Left Mouse Button / Mouse Wheel) */
	UFUNCTION(BlueprintCallable, Category = "CRT Interaction")
	void OnCRTLeftClickPressed();

	UFUNCTION(BlueprintCallable, Category = "CRT Interaction")
	void OnCRTLeftClickReleased();

	UFUNCTION(BlueprintCallable, Category = "CRT Interaction")
	void OnCRTScrollWheel(float ScrollDelta);

	// --- Paramètres de calibration UV réglables dans les détails du Blueprint ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT Interaction")
	int32 UVChannel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT Interaction")
	bool bFlipU = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT Interaction")
	bool bFlipV = false;

	/** Affiche le point Rouge (vitre bombée) et le point Vert (plan du Widget). Passe-le à false quand tout est calé. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT Interaction")
	bool bDrawDebugCRT = false;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PCMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* TriggerBox;

	// Le widget pour afficher la touche "E"
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UWidgetComponent* PromptWidget;

	// Le composant qui simule le pointeur de souris sur l'UI à partir des UVs
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UWidgetInteractionComponent* CRTWidgetInteraction;
	
	bool bIsPlayerNear;

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnPCInteracted();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void ShowPCInterface();

private:
	UPROPERTY()
	class UStaticMeshComponent* CRTScreenMesh;

	UPROPERTY()
	class UWidgetComponent* CRTWidgetComp;

	bool bIsLeftMousePressed = false;
	FHitResult LastValidCustomHit;
};