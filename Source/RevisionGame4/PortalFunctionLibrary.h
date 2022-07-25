// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "PortalFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class REVISIONGAME4_API UPortalFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category="Portal")
		static void ResizeRenderTarget(UTextureRenderTarget2D* render_target, float size_x, float size_y);
	
};
