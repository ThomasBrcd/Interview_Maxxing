#include "InterviewGameUserSettings.h"
#include "Engine/Engine.h"

UInterviewGameUserSettings* UInterviewGameUserSettings::GetInterviewGameUserSettings()
{
	return GEngine ? Cast<UInterviewGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}

void UInterviewGameUserSettings::SetSensibiliteSouris(float NouvelleSensibilite)
{
	SensibiliteSouris = FMath::Clamp(NouvelleSensibilite, SensibiliteMin, SensibiliteMax);
}

void UInterviewGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();
	SensibiliteSouris = 1.0f;
}
