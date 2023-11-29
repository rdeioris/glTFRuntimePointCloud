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
		return nullptr;
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

ULidarPointCloud* UglTFRuntimePointCloudLibrary::LoadPointCloudFromXYZ(UglTFRuntimeAsset* Asset, const FglTFRuntimeASCIIPointCloudConfig& ASCIIPointCloudConfig)
{
	if (!Asset)
	{
		return nullptr;
	}

	const TArray64<uint8>& Blob = Asset->GetParser()->GetBlob();

	TArray<FLidarPointCloudPoint> Points;

	TArray<TArray<FString>> Lines;
	TArray<FString> CurrentLine;
	FString CurrentString;

	for (int64 Index = 0; Index < Blob.Num(); Index++)
	{
		const char Char = static_cast<char>(Blob[Index]);
		if (Char == ' ' || Char == '\t' || Char == '\r' || Char == '\n')
		{
			if (!CurrentString.IsEmpty())
			{
				CurrentLine.Add(CurrentString);
			}
			CurrentString = "";
			if (Char == '\r' || Char == '\n')
			{
				if (CurrentLine.Num() > 0)
				{
					Lines.Add(CurrentLine);
				}
				CurrentLine.Empty();
			}
		}
		else
		{
			CurrentString += Char;
		}
	}

	const FglTFRuntimeASCIIPointCloudConfig& Config = ASCIIPointCloudConfig;

	if (Config.LinesToSkip < 0 || Config.LinesToSkip > Lines.Num())
	{
		return nullptr;
	}

	for (int32 LineIndex = Config.LinesToSkip; LineIndex < Lines.Num(); LineIndex++)
	{
		const TArray<FString>& Line = Lines[LineIndex];

		FLidarPointCloudPoint Point;

		if (Config.XYZColumns.X >= 0 && Config.XYZColumns.X < Line.Num())
		{
			Point.Location.X = FCString::Atof(*(Line[Config.XYZColumns.X]));
		}

		if (Config.XYZColumns.Y >= 0 && Config.XYZColumns.Y < Line.Num())
		{
			Point.Location.Y = FCString::Atof(*(Line[Config.XYZColumns.Y]));
		}

		if (Config.XYZColumns.Z >= 0 && Config.XYZColumns.Z < Line.Num())
		{
			Point.Location.Z = FCString::Atof(*(Line[Config.XYZColumns.Z]));
		}

		if (Config.RGBColumns.X >= 0 && Config.RGBColumns.X < Line.Num())
		{
			Point.Color.R = FCString::Atof(*(Line[Config.RGBColumns.X])) * (Config.bFloatColors ? 255 : 1);
		}

		if (Config.RGBColumns.Y >= 0 && Config.RGBColumns.Y < Line.Num())
		{
			Point.Color.G = FCString::Atof(*(Line[Config.RGBColumns.Y])) * (Config.bFloatColors ? 255 : 1);
		}

		if (Config.RGBColumns.Z >= 0 && Config.RGBColumns.Z < Line.Num())
		{
			Point.Color.B = FCString::Atof(*(Line[Config.RGBColumns.Z])) * (Config.bFloatColors ? 255 : 1);
		}

		if (Config.NormalColumns.X >= 0 && Config.NormalColumns.X < Line.Num())
		{
			Point.Normal.X = FCString::Atof(*(Line[Config.NormalColumns.X]));
		}

		if (Config.NormalColumns.Y >= 0 && Config.NormalColumns.Y < Line.Num())
		{
			Point.Normal.Y = FCString::Atof(*(Line[Config.NormalColumns.Y]));
		}

		if (Config.NormalColumns.Z >= 0 && ASCIIPointCloudConfig.NormalColumns.Z < Line.Num())
		{
			Point.Normal.Z = FCString::Atof(*(Line[Config.NormalColumns.Z]));
		}

		Points.Add(MoveTemp(Point));
	}


	return ULidarPointCloud::CreateFromData(Points, false);
}