#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NotificationEcran.generated.h"

class UTextBlock;

/**
 * Bandeau d'information affiché quelques secondes en haut de l'écran
 * (ex : « Terminez votre test technique avant de partir »).
 * L'interface est construite en C++ : aucun asset n'est nécessaire.
 */
UCLASS()
class INTERVIEW_MAXXING_API UNotificationEcran : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Affiche le message pendant Duree secondes (remplace la notification précédente) */
	UFUNCTION(BlueprintCallable, Category = "Boucle de jeu", meta = (WorldContext = "WorldContextObject", Duree = "3.0"))
	static void AfficherNotification(UObject* WorldContextObject, FText Message, float Duree = 3.f);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	void ConstruireInterface();
	void Fermer();

	UPROPERTY()
	TObjectPtr<UTextBlock> Texte;

	FTimerHandle TimerFermeture;

	static TWeakObjectPtr<UNotificationEcran> NotificationActuelle;
};
