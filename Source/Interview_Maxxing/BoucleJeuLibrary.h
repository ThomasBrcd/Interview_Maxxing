#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BoucleJeuLibrary.generated.h"

class UPanelWidget;

/** Fonctions utilitaires de la boucle de jeu (chambre -> entretien -> test technique -> chambre) */
UCLASS()
class INTERVIEW_MAXXING_API UBoucleJeuLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Quitte l'écran d'un ordinateur : la caméra revient sur le personnage,
	 * la souris est cachée et les déplacements sont rendus au joueur.
	 */
	UFUNCTION(BlueprintCallable, Category = "Boucle de jeu", meta = (WorldContext = "WorldContextObject"))
	static void RendreControleAuJoueur(UObject* WorldContextObject);

	/**
	 * Retire de la liste des mails les offres auxquelles le joueur a déjà postulé
	 * (variable ID_Mail des éléments comparée à OffresPostulees du GameInstance).
	 */
	UFUNCTION(BlueprintCallable, Category = "Boucle de jeu", meta = (WorldContext = "WorldContextObject"))
	static void RetirerOffresPostulees(UObject* WorldContextObject, UPanelWidget* ListeMails);
};
