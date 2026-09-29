
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

	UFUNCTION(BlueprintCallable, Category="Interaction")
	void InteractWithPC();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PCMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* TriggerBox;
	
	bool bIsPlayerNear;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void ShowPCInterface();
};
