// Fill out your copyright notice in the Description page of Project Settings.


#include "Arcweave_BP.h"
#include "Arcweave.h"

bool UArcweave_BP::ArcweaveTest()
{
	FArcweaveModule& ArcweaveModule = FModuleManager::LoadModuleChecked<FArcweaveModule>("Arcweave");
	return ArcweaveModule.TestJsonFile();
}
