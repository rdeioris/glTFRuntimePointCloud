// Copyright 2022, Roberto De Ioris.

#pragma once

#include "CoreMinimal.h"
#include "glTFRuntimeAsset.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LidarPointCloud.h"
#include "glTFRuntimePointCloudLibrary.generated.h"

USTRUCT(BlueprintType)
struct FglTFRuntimeASCIIPointCloudConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTFRuntime|PointCloud")
	FIntVector XYZColumns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTFRuntime|PointCloud")
	FIntVector RGBColumns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTFRuntime|PointCloud")
	FIntVector NormalColumns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTFRuntime|PointCloud")
	int32 AlphaColumn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTFRuntime|PointCloud")
	int32 LinesToSkip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTFRuntime|PointCloud")
	bool bFloatColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTFRuntime|PointCloud")
	bool bComputeColumnsMinMax;

	FglTFRuntimeASCIIPointCloudConfig()
	{
		XYZColumns.X = 0;
		XYZColumns.Y = 1;
		XYZColumns.Z = 2;

		RGBColumns.X = 3;
		RGBColumns.Y = 4;
		RGBColumns.Z = 5;

		NormalColumns.X = 6;
		NormalColumns.Y = 7;
		NormalColumns.Z = 8;

		AlphaColumn = 9;

		LinesToSkip = 0;

		bFloatColors = false;

		bComputeColumnsMinMax = false;
	}
};

/**
 *
 */
UCLASS()
class GLTFRUNTIMEPOINTCLOUD_API UglTFRuntimePointCloudLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "glTFRuntime|PointCloud")
	static bool HasPointCloud(UglTFRuntimeAsset* Asset, const int32 MeshIndex);

	UFUNCTION(BlueprintCallable, Category = "glTFRuntime|PointCloud")
	static ULidarPointCloud* LoadPointCloudFromMesh(UglTFRuntimeAsset* Asset, const int32 MeshIndex);

	UFUNCTION(BlueprintCallable, Category = "glTFRuntime|PointCloud")
	static ULidarPointCloud* LoadPointCloudFromMeshes(UglTFRuntimeAsset* Asset, const TArray<int32>& MeshIndices);

	UFUNCTION(BlueprintCallable, Category = "glTFRuntime|PointCloud", meta = (AutoCreateRefTerm = "ASCIIPointCloudConfig"))
	static ULidarPointCloud* LoadPointCloudFromXYZ(UglTFRuntimeAsset* Asset, const FglTFRuntimeASCIIPointCloudConfig& ASCIIPointCloudConfig);

	UFUNCTION(BlueprintCallable, Category = "glTFRuntime|PointCloud", meta = (AutoCreateRefTerm = "ASCIIPointCloudConfig"))
	static ULidarPointCloud* LoadPointCloudFromPCD(UglTFRuntimeAsset* Asset, FTransform& ViewPoint);

	static ULidarPointCloud* LoadPointCloudFromXYZWithFilter(UglTFRuntimeAsset* Asset, TFunction<void(FLidarPointCloudPoint&, const TArray<FString>&, const FglTFRuntimeASCIIPointCloudConfig&) > StringFilter, TFunction<void(FLidarPointCloudPoint&, const TArray<double>&, const TArray<double>&, const TArray<double>&, const FglTFRuntimeASCIIPointCloudConfig&)> FloatFilter, const FglTFRuntimeASCIIPointCloudConfig& ASCIIPointCloudConfig);

};
