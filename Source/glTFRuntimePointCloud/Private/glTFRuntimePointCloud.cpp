// Copyright 2022, Roberto De Ioris.

#include "glTFRuntimePointCloud.h"

#define LOCTEXT_NAMESPACE "FglTFRuntimePointCloudModule"

void FglTFRuntimePointCloudModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FglTFRuntimePointCloudModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FglTFRuntimePointCloudModule, glTFRuntimePointCloud)