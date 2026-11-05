#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "InterviewGameUserSettings.generated.h"

/**
 * Réglages joueur sauvegardés dans GameUserSettings.ini :
 * ceux d'Unreal (mode d'affichage, résolution...) + la sensibilité de la souris.
 */
UCLASS()
class INTERVIEW_MAXXING_API UInterviewGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	static constexpr float SensibiliteMin = 0.1f;
	static constexpr float SensibiliteMax = 3.0f;

	/** Renvoie les réglages du jeu (classe définie dans DefaultEngine.ini) */
	UFUNCTION(BlueprintCallable, Category = "Parametres")
	static UInterviewGameUserSettings* GetInterviewGameUserSettings();

	UFUNCTION(BlueprintCallable, Category = "Parametres")
	void SetSensibiliteSouris(float NouvelleSensibilite);

	UFUNCTION(BlueprintPure, Category = "Parametres")
	float GetSensibiliteSouris() const { return SensibiliteSouris; }

	virtual void SetToDefaults() override;

protected:
	/** Multiplicateur appliqué à la rotation de la caméra (1 = vitesse d'origine) */
	UPROPERTY(Config)
	float SensibiliteSouris = 1.0f;
};
