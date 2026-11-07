#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GenericPlatform/GenericWindow.h"
#include "MenuParametres.generated.h"

class UComboBoxString;
class USlider;
class UTextBlock;
class UButton;
class UVerticalBox;
class UWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuParametresFerme);

/**
 * Menu des paramètres : mode d'affichage (plein écran / sans bordure / fenêtré),
 * résolution et sensibilité de la souris.
 * L'interface est construite en C++ : aucun asset n'est nécessaire.
 */
UCLASS()
class INTERVIEW_MAXXING_API UMenuParametres : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Appelé quand le joueur ferme le menu (Retour ou Échap) */
	UPROPERTY(BlueprintAssignable, Category = "Parametres")
	FOnMenuParametresFerme OnFerme;

	UFUNCTION(BlueprintCallable, Category = "Parametres")
	void Fermer();

	/** Affiche le bouton « Menu principal » (menu ouvert en cours de partie) */
	void SetProposerMenuPrincipal(bool bProposer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	void ConstruireInterface();
	UTextBlock* CreerTexte(const FText& Texte, int32 Taille, bool bGras = false);
	UButton* CreerBouton(const FText& Libelle);
	void AjouterLigne(UVerticalBox* Colonne, const FText& Libelle, UWidget* Controle);

	void ChargerValeurs();
	void RemplirResolutions();
	void MettreAJourTexteSensibilite();
	EWindowMode::Type GetModeSelectionne() const;

	UFUNCTION()
	void OnModeChange(FString Option, ESelectInfo::Type TypeSelection);

	UFUNCTION()
	void OnSensibiliteChange(float Valeur);

	UFUNCTION()
	void OnAppliquer();

	UFUNCTION()
	void OnRetour();

	UFUNCTION()
	void OnMenuPrincipal();

	UPROPERTY()
	TObjectPtr<UButton> BoutonMenuPrincipal;

	UPROPERTY()
	TObjectPtr<UComboBoxString> ComboMode;

	UPROPERTY()
	TObjectPtr<UComboBoxString> ComboResolution;

	UPROPERTY()
	TObjectPtr<USlider> SliderSensibilite;

	UPROPERTY()
	TObjectPtr<UTextBlock> TexteSensibilite;

	UPROPERTY()
	TObjectPtr<UTextBlock> TexteAide;

	/** Résolutions proposées, dans le même ordre que ComboResolution */
	TArray<FIntPoint> Resolutions;
};
