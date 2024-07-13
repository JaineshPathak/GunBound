// Fill out your copyright notice in the Description page of Project Settings.


#include "GetProjectVersion.h"

FString UGetProjectVersion::GetProjectVersion()
{
    FString ProjectVersion;
    GConfig->GetString(
        TEXT("/Script/EngineSettings.GeneralProjectSettings"),
        TEXT("ProjectVersion"),
        ProjectVersion,
        GGameIni
    );

    return ProjectVersion;
}
