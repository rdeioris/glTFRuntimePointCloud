// Copyright 2022-2023, Roberto De Ioris.

#include "glTFRuntimePointCloudLibrary.h"
#include "Async/ParallelFor.h"

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

	const int32 NumLines = Lines.Num() - Config.LinesToSkip;

	Points.AddUninitialized(NumLines);

	ParallelFor(NumLines, [&](const int32 LineIndexOffset)
		{
			const int32 LineIndex = LineIndexOffset + Config.LinesToSkip;

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

			if (Config.AlphaColumn >= 0 && ASCIIPointCloudConfig.AlphaColumn < Line.Num())
			{
				Point.Color.A = FCString::Atof(*(Line[Config.AlphaColumn])) * (Config.bFloatColors ? 255 : 1);
			}

			Points[LineIndexOffset] = MoveTemp(Point);
		});


	return ULidarPointCloud::CreateFromData(Points, false);
}

ULidarPointCloud* UglTFRuntimePointCloudLibrary::LoadPointCloudFromPCD(UglTFRuntimeAsset* Asset, FTransform& ViewPoint)
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

	int64 BinaryIndex = -1;
	int64 ASCIIIndex = -1;
	bool bBinaryCompressed = false;

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
					if (CurrentLine.Num() >= 2 && CurrentLine[0] == "DATA")
					{
						ASCIIIndex = Lines.Num();

						if (CurrentLine[1] != "ascii")
						{
							BinaryIndex = Index + 1;
							bBinaryCompressed = CurrentLine[1] == "binary_compressed";
							break;
						}
					}
				}
				CurrentLine.Empty();
			}
		}
		else
		{
			CurrentString += Char;
		}
	}

	if (ASCIIIndex < 0)
	{
		return nullptr;
	}

	TMap<FString, TArray<FString>> HeaderFields;

	for (int64 Index = 0; Index < ASCIIIndex; Index++)
	{
		HeaderFields.Add(Lines[Index][0], Lines[Index]);
	}

	int64 NumberOfFields = 0;
	FIntVector XYZ = { -1, -1, -1 };
	int32 RGB = -1;
	FIntVector NXYZ = { -1, -1, -1 };

	int64 Width = 0;
	int64 Height = 1;
	int64 NumberOfPoints = 0;

	int64 ChunkSize = 0;

	if (HeaderFields.Contains("FIELDS"))
	{
		const TArray<FString>& Fields = HeaderFields["FIELDS"];
		for (int32 Index = 1; Index < Fields.Num(); Index++)
		{
			const FString& Field = Fields[Index];
			if (Field == "x")
			{
				XYZ.X = Index - 1;
			}
			else if (Field == "y")
			{
				XYZ.Y = Index - 1;
			}
			else if (Field == "z")
			{
				XYZ.Z = Index - 1;
			}
			else if (Field == "rgb")
			{
				RGB = Index - 1;
			}
			else if (Field == "normal_x")
			{
				NXYZ.X = Index - 1;
			}
			else if (Field == "normal_y")
			{
				NXYZ.Y = Index - 1;
			}
			else if (Field == "normal_z")
			{
				NXYZ.Z = Index - 1;
			}
		}

		NumberOfFields = Fields.Num() - 1;
	}

	TMap<int32, int64> BinaryOffsetsMap;
	TMap<int32, int64> BinarySizeMap;
	if (HeaderFields.Contains("SIZE"))
	{
		const TArray<FString>& Fields = HeaderFields["SIZE"];
		for (int32 Index = 1; Index < Fields.Num(); Index++)
		{
			int64 Size = FCString::Atoi64(*(Fields[Index]));
			if (HeaderFields.Contains("COUNT"))
			{
				const TArray<FString>& CountFields = HeaderFields["COUNT"];
				if (CountFields.IsValidIndex(Index))
				{
					Size *= FCString::Atoi64(*(CountFields[Index]));
				}
			}
			BinaryOffsetsMap.Add(Index - 1, ChunkSize);
			BinarySizeMap.Add(Index - 1, Size);
			ChunkSize += Size;
		}
	}

	if (HeaderFields.Contains("WIDTH"))
	{
		const TArray<FString>& Fields = HeaderFields["WIDTH"];
		if (Fields.Num() > 1)
		{
			Width = FCString::Atoi64(*(Fields[1]));
		}
	}

	if (HeaderFields.Contains("HEIGHT"))
	{
		const TArray<FString>& Fields = HeaderFields["HEIGHT"];
		if (Fields.Num() > 1)
		{
			Height = FCString::Atoi64(*(Fields[1]));
		}
	}

	if (HeaderFields.Contains("POINTS"))
	{
		const TArray<FString>& Fields = HeaderFields["POINTS"];
		if (Fields.Num() > 1)
		{
			NumberOfPoints = FCString::Atoi64(*(Fields[1]));
		}
	}

	if (NumberOfPoints < (Width * Height))
	{
		NumberOfPoints = Width * Height;
	}

	if (BinaryIndex > -1)
	{
		const uint8* DataPtr = Blob.GetData() + BinaryIndex;
		int64 DataLen = Blob.Num() - BinaryIndex;

		TArray64<uint8> Data2; // just keep it for eventual reordering after decompression

		if (bBinaryCompressed)
		{
			if (DataLen < 8)
			{
				return nullptr;
			}

			const uint32* CompressedSize = reinterpret_cast<const uint32*>(DataPtr);
			const uint32* UncompressedSize = reinterpret_cast<const uint32*>(DataPtr + 4);

			if (*CompressedSize > DataLen - 8)
			{
				return nullptr;
			}

			TArray64<uint8> LZFOutput;
			LZFOutput.AddUninitialized(*UncompressedSize);

			DataPtr += 8;
			int64 InputOffset = 0;
			int64 OutputOffset = 0;

			while (InputOffset < *CompressedSize)
			{
				uint8 Ctrl = DataPtr[InputOffset++];
				if (Ctrl < (1 << 5)) // Mode 0
				{
					Ctrl++;
					if (OutputOffset + Ctrl > *UncompressedSize)
					{
						return nullptr;
					}
					if (InputOffset + Ctrl > *CompressedSize)
					{
						return nullptr;
					}

					FMemory::Memcpy(LZFOutput.GetData() + OutputOffset, DataPtr + InputOffset, Ctrl);
					InputOffset += Ctrl;
					OutputOffset += Ctrl;
				}
				else
				{
					uint32 Length = Ctrl >> 5;
					int32 BackReferenceOffset = OutputOffset - (((uint32)(Ctrl & 0x1f)) << 8) - 1;

					if (InputOffset >= *CompressedSize)
					{
						return nullptr;
					}

					if (Length == 7)
					{
						Length += DataPtr[InputOffset++];

						if (InputOffset >= *CompressedSize)
						{
							return nullptr;
						}
					}

					BackReferenceOffset -= DataPtr[InputOffset++];
					if (BackReferenceOffset < 0)
					{
						return nullptr;
					}

					if (OutputOffset + Length + 2 > *UncompressedSize)
					{
						return nullptr;
					}

					LZFOutput[OutputOffset++] = LZFOutput[BackReferenceOffset++];

					LZFOutput[OutputOffset++] = LZFOutput[BackReferenceOffset++];

					while (Length > 0)
					{
						LZFOutput[OutputOffset++] = LZFOutput[BackReferenceOffset++];
						Length--;
					}
				}
			}

			// check decompressed binary size
			if (LZFOutput.Num() < NumberOfPoints * ChunkSize)
			{
				return nullptr;
			}

			Data2.AddUninitialized(LZFOutput.Num());

			for (int64 FieldIndex = 0; FieldIndex < NumberOfFields; FieldIndex++)
			{
				for (int64 PointIndex = 0; PointIndex < NumberOfPoints; PointIndex++)
				{
					const int32 Size = BinarySizeMap[FieldIndex];
					const int64 SrcOffset = FieldIndex * (Size * NumberOfPoints) + (PointIndex * Size);
					const int64 DstOffset = PointIndex * ChunkSize + BinaryOffsetsMap[FieldIndex];
					FMemory::Memcpy(Data2.GetData() + DstOffset, LZFOutput.GetData() + SrcOffset, Size);
				}
			}

			DataPtr = Data2.GetData();
			DataLen = Data2.Num();
		}

		// check binary size
		if (DataLen < NumberOfPoints * ChunkSize)
		{
			return nullptr;
		}

		// check binary offsets
		if (BinaryOffsetsMap.Num() != NumberOfFields)
		{
			return nullptr;
		}

		for (int64 Index = 0; Index < NumberOfPoints; Index++)
		{
			FLidarPointCloudPoint Point;
			const uint8* PointPtr = DataPtr + (Index * ChunkSize);

			if (XYZ.X > -1)
			{
				const float* Ptr = reinterpret_cast<const float*>(PointPtr + BinaryOffsetsMap[XYZ.X]);
				Point.Location.X = *Ptr;
			}
			if (XYZ.Y > -1)
			{
				const float* Ptr = reinterpret_cast<const float*>(PointPtr + BinaryOffsetsMap[XYZ.Y]);
				Point.Location.Y = *Ptr;
			}
			if (XYZ.Z > -1)
			{
				const float* Ptr = reinterpret_cast<const float*>(PointPtr + BinaryOffsetsMap[XYZ.Z]);
				Point.Location.Z = *Ptr;
			}
			if (RGB > -1)
			{
				const uint32* Ptr = reinterpret_cast<const uint32*>(PointPtr + BinaryOffsetsMap[RGB]);
				Point.Color.R = (*Ptr >> 16) & 0xFF;
				Point.Color.G = (*Ptr >> 8) & 0xFF;
				Point.Color.B = (*Ptr) & 0xFF;
			}

			Points.Add(MoveTemp(Point));
		}
	}
	else
	{

	}

	return ULidarPointCloud::CreateFromData(Points, false);
}