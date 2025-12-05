// Fill out your copyright notice in the Description page of Project Settings.
#include "DiceActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ChildActorComponent.h"

ADiceActor::ADiceActor()
{
    PrimaryActorTick.bCanEverTick = false;

    DiceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DiceMesh"));
    RootComponent = DiceMesh;
    
    DiceMesh->SetSimulatePhysics(true);
    DiceMesh->SetEnableGravity(true);
    DiceMesh->SetAngularDamping(2.0f);
    DiceMesh->SetLinearDamping(1.0f);
}

void ADiceActor::UpdateDiceFaces()
{
    if (!DiceMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateDiceFaces: DiceMesh is null!"));
        return;
    }

    // Get child components
    TArray<USceneComponent*> AttachedChildren = DiceMesh->GetAttachChildren();
    
    for (int i = 0; i < AttachedChildren.Num(); ++i)
    {
        USceneComponent* Child = AttachedChildren[i];
        if (!Child)
            continue;

        // Cast to ChildActorComponent
        UChildActorComponent* ChildActorComp = Cast<UChildActorComponent>(Child);
        if (!ChildActorComp)
        {
            UE_LOG(LogTemp, Warning, TEXT("Child %d is not a ChildActorComponent"), i);
            continue;
        }

        // Get the spawned child actor (BP_WorldFace)
        AActor* ChildActor = ChildActorComp->GetChildActor();
        if (!ChildActor)
        {
            UE_LOG(LogTemp, Warning, TEXT("ChildActorComponent %d has no child actor spawned"), i);
            continue;
        }

        // Get the face values for this index
        FDiceFaceValues FaceValues = DiceFaces.GetFaceValues(i);

        // Find UpdateText function on the child actor
        UFunction* UpdateTextFunc = ChildActor->FindFunction(FName("UpdateText"));
        
        if (UpdateTextFunc)
        {
            
            // Allocate proper parameter buffer
            uint8* Params = (uint8*)FMemory_Alloca(UpdateTextFunc->ParmsSize);
            FMemory::Memzero(Params, UpdateTextFunc->ParmsSize);
            
            // Copy the FaceValues to the parameter
            for (TFieldIterator<FProperty> PropIt(UpdateTextFunc); PropIt; ++PropIt)
            {
                FProperty* Prop = *PropIt;
                if (Prop->HasAnyPropertyFlags(CPF_Parm) && !Prop->HasAnyPropertyFlags(CPF_ReturnParm))
                {
                    FStructProperty* StructProp = CastField<FStructProperty>(Prop);
                    if (StructProp && StructProp->Struct == FDiceFaceValues::StaticStruct())
                    {
                        void* ValuePtr = StructProp->ContainerPtrToValuePtr<void>(Params);
                        StructProp->CopyCompleteValue(ValuePtr, &FaceValues);
                    }
                }
            }

            ChildActor->ProcessEvent(UpdateTextFunc, Params);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("UpdateText function NOT found on actor %d"), i);
        }
    }
}


void ADiceActor::BeginPlay()
{
    Super::BeginPlay();
}

void ADiceActor::RollDice()
{
    if (DiceMesh)
    {
        FVector RandomForce = FVector(
            FMath::RandRange(-RollForce, RollForce),
            FMath::RandRange(-RollForce, RollForce),
            FMath::RandRange(RollForce * 0.5f, RollForce)
        );
        
        FVector RandomTorque = FVector(
            FMath::RandRange(-RollTorque, RollTorque),
            FMath::RandRange(-RollTorque, RollTorque),
            FMath::RandRange(-RollTorque, RollTorque)
        );
        
        DiceMesh->AddImpulse(RandomForce, NAME_None, true);
        DiceMesh->AddTorqueInRadians(RandomTorque, NAME_None, true);
        
        bIsRolling = true;
        GetWorldTimerManager().SetTimer(StabilityCheckTimer, this, &ADiceActor::CheckIfStable, 0.1f, true);
    }
}

void ADiceActor::CheckIfStable()
{
    if (DiceMesh->GetPhysicsLinearVelocity().Size() < 10.0f && 
        DiceMesh->GetPhysicsAngularVelocityInDegrees().Size() < 10.0f)
    {
        bIsRolling = false;
        GetWorldTimerManager().ClearTimer(StabilityCheckTimer);
        FDiceFaceValues Result = GetResultingFace();
        OnRollComplete.Broadcast(Result);
        OnDiceRollComplete(Result);
    }
}

int ADiceActor::GetTopFaceIndex()
{
    // Prefer using child components' X axes as face normals (child 0 => face 0)
    if (DiceMesh)
    {
        TArray<USceneComponent*> AttachedChildren = DiceMesh->GetAttachChildren();
        if (AttachedChildren.Num() > 0)
        {
            int BestFaceIndex = 0;
            float BestDot = -1.0f;

            for (int i = 0; i < AttachedChildren.Num(); ++i)
            {
                USceneComponent* Child = AttachedChildren[i];
                if (!Child) continue;

                // Use the child's X axis in world-space as the face normal
                FVector WorldX = Child->GetComponentTransform().GetUnitAxis(EAxis::X);
                float DotProduct = FVector::DotProduct(WorldX, FVector::UpVector);

                if (DotProduct > BestDot)
                {
                    BestDot = DotProduct;
                    BestFaceIndex = i;
                }
            }

            return BestFaceIndex;
        }
    }

    // Fallback to previous hardcoded normals if no children are attached or DiceMesh is null
    TArray<FVector> FaceNormals = {
        FVector(0, 0, 1),   // Face 0 (top)
        FVector(0, 0, -1),  // Face 1 (bottom)
        FVector(1, 0, 0),   // Face 2 (right)
        FVector(-1, 0, 0),  // Face 3 (left)
        FVector(0, 1, 0),   // Face 4 (front)
        FVector(0, -1, 0)   // Face 5 (back)
    };

    int BestFaceIndex = 0;
    float BestDot = -1.0f;

    for (int i = 0; i < FaceNormals.Num(); i++)
    {
        FVector WorldNormal = DiceMesh->GetComponentRotation().RotateVector(FaceNormals[i]);
        float DotProduct = FVector::DotProduct(WorldNormal, FVector::UpVector);

        if (DotProduct > BestDot)
        {
            BestDot = DotProduct;
            BestFaceIndex = i;
        }
    }

    return BestFaceIndex;
}


FDiceFaceValues ADiceActor::GetResultingFace()
{
    return DiceFaces.GetFaceValues(GetTopFaceIndex());
}
