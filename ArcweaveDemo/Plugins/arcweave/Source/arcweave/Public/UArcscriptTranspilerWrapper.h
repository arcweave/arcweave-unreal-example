// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArcscriptTranspiler.h"
#include "UObject/Object.h"
#include "UArcscriptTranspilerWrapper.generated.h"

/**
 * 
 */
UCLASS()
class ARCWEAVE_API UArcscriptTranspilerWrapper : public UObject
{
	GENERATED_BODY()

public:
	
	UArcscriptTranspilerWrapper(){};

private:
	Arcweave::ArcscriptTranspiler* Transpiler;

	// Additional helper methods can be added here if needed.
};	

