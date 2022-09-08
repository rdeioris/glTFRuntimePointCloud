// Copyright 2022, Roberto De Ioris.


#include "glTFRuntimePointCloudLibrary.h"

bool UglTFRuntimePointCloudLibrary::HasPointCloud(UglTFRuntimeAsset* Asset, const int32 MeshIndex)
{
	TSharedPtr<FJsonObject> JsonMeshObject = Asset->GetParser()->GetJsonObjectFromRootIndex("meshes", MeshIndex);
	if (!JsonMeshObject)
	{
		return false;
	}

	for (TSharedRef<FJsonObject> JsonPrimitive : Asset->GetParser()->GetJsonObjectArrayOfObjects(JsonMeshObject.ToSharedRef(), "primitives"))
	{
		if (Asset->GetParser()->GetJsonObjectNumber(JsonPrimitive, "mode", 4) == 0) // 4 is the default for triangles
		{
			return true;
		}
	}

	return false;
}

ULidarPointCloud* UglTFRuntimePointCloudLibrary::LoadPointCloudFromMesh(UglTFRuntimeAsset* Asset, const int32 MeshIndex)
{
	TSharedPtr<FJsonObject> JsonMeshObject = Asset->GetParser()->GetJsonObjectFromRootIndex("meshes", MeshIndex);
	if (!JsonMeshObject)
	{
		return nullptr;
	}

	TArray<FglTFRuntimePrimitive> Primitives;
	if (!Asset->GetParser()->LoadPrimitives(JsonMeshObject.ToSharedRef(), Primitives, FglTFRuntimeMaterialsConfig()))
	{
		return false;
	}

	TArray<FLidarPointCloudPoint> Points;


	for (const FglTFRuntimePrimitive& Primitive : Primitives)
	{
		if (Primitive.Mode == 0)
		{
			for (uint32 Index : Primitive.Indices)
			{
				FLidarPointCloudPoint Point;
				if (Primitive.Positions.IsValidIndex(Index))
				{
					Point.Location = FVector3f(Primitive.Positions[Index]);
				}
				if (Primitive.Colors.IsValidIndex(Index))
				{
					Point.Color = FLinearColor(Primitive.Colors[Index]).ToFColor(true);
				}
				if (Primitive.Normals.IsValidIndex(Index))
				{
					Point.Normal = FVector3f(Primitive.Normals[Index]);
				}

				Points.Add(MoveTemp(Point));
			}
		}
	}

	return ULidarPointCloud::CreateFromData(Points, false);
}

ULidarPointCloud* UglTFRuntimePointCloudLibrary::LoadPointCloudFromMeshes(UglTFRuntimeAsset* Asset, const TArray<int32>& MeshIndices)
{
	TArray<FLidarPointCloudPoint> Points;

	for (const int32 MeshIndex : MeshIndices)
	{
		TSharedPtr<FJsonObject> JsonMeshObject = Asset->GetParser()->GetJsonObjectFromRootIndex("meshes", MeshIndex);
		if (!JsonMeshObject)
		{
			continue;
		}

		TArray<FglTFRuntimePrimitive> Primitives;
		if (!Asset->GetParser()->LoadPrimitives(JsonMeshObject.ToSharedRef(), Primitives, FglTFRuntimeMaterialsConfig()))
		{
			continue;
		}

		for (const FglTFRuntimePrimitive& Primitive : Primitives)
		{
			if (Primitive.Mode == 0)
			{
				for (uint32 Index : Primitive.Indices)
				{
					FLidarPointCloudPoint Point;
					if (Primitive.Positions.IsValidIndex(Index))
					{
						Point.Location = FVector3f(Primitive.Positions[Index]);
					}
					if (Primitive.Colors.IsValidIndex(Index))
					{
						Point.Color = FLinearColor(Primitive.Colors[Index]).ToFColor(true);
					}
					if (Primitive.Normals.IsValidIndex(Index))
					{
						Point.Normal = FVector3f(Primitive.Normals[Index]);
					}

					Points.Add(MoveTemp(Point));
				}
			}
		}
	}

	return ULidarPointCloud::CreateFromData(Points, false);
}