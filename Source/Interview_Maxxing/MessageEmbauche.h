#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MessageEmbauche.generated.h"

class UTextBlock;
class UButton;

/**
 * Message reçu en rentrant dans la chambre après le test technique :
 * l'entreprise annonce si le joueur est embauché ou non.
 * L'interface est construite en C++ : aucun asset n'est nécessaire.
 */
UCLASS()
class INTERVIEW_MAXXING_API UMessageEmbauche : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Affiche le message de l'entreprise, joue le son de résultat et donne la souris au joueur.
	 * @param bEmbauche  true si le test technique est réussi
	 * @param IdOffre    nom de la ligne de DT_Offres_Mails (sert à retrouver le nom de l'entreprise)
	 */
	UFUNCTION(BlueprintCallable, Category = "Boucle de jeu", meta = (WorldContext = "WorldContextObject"))
	static UMessageEmbauche* AfficherMessageEmbauche(UObject* WorldContextObject, bool bEmbauche, FName IdOffre);

	/** Ferme le message et rend le contrôle du personnage au joueur */
	UFUNCTION(BlueprintCallable, Category = "Boucle de jeu")
	void Fermer();

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	void ConstruireInterface();
	void Configurer(bool bEmbauche, const FText& NomEntreprise);

	UFUNCTION()
	void OnContinuer();

	UPROPERTY()
	TObjectPtr<UTextBlock> TexteTitre;

	UPROPERTY()
	TObjectPtr<UTextBlock> TexteMessage;

	UPROPERTY()
	TObjectPtr<UButton> BoutonContinuer;
};
