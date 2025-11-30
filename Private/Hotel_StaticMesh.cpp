// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_StaticMesh.h"
#include "Hotel_Manager.h"

void UHotel_StaticMesh::BeginPlay()
{
   Super::BeginPlay();
   GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>()->AddHotelMesh(MeshName,this);
   UE_LOG(LogTemp, Warning, TEXT("%s 벽로드 오류"), *MeshName.ToString());
}
void UHotel_StaticMesh::SetMeshState(bool state)
{
    SetVisibility(state);
    FName TempName;
    state ? TempName = "BlockAllDynamic" : TempName = "NoCollision";
    SetCollisionProfileName(TempName);
}