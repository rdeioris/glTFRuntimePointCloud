// Copyright 2022, Roberto De Ioris.

#pragma once

#include "CoreMinimal.h"
#include "glTFRuntimeAsset.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LidarPointCloud.h"
#include "glTFRuntimePointCloudLibrary.generated.h"

/**
 * 
 */
UCLASS()
class GLTFRUNTIMEPOINTCLOUD_API UglTFRuntimePointCloudLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static bool HasPointCloud(UglTFRuntimeAsset* Asset, const int32 MeshIndex);

	UFUNCTION(BlueprintCallable)
	static ULidarPointCloud* LoadPointCloudFromMesh(UglTFRuntimeAsset* Asset, const int32 MeshIndex);

	UFUNCTION(BlueprintCallable)
	static ULidarPointCloud* LoadPointCloudFromMeshes(UglTFRuntimeAsset* Asset, const TArray<int32>& MeshIndices);
	
};
