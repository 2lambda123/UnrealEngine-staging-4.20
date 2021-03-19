// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
//
#include "SteamVRFunctionLibrary.h"
#include "SteamVRPrivate.h"
#include "SteamVRHMD.h"
#include "XRMotionControllerBase.h"

USteamVRFunctionLibrary::USteamVRFunctionLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{ 
}

#if STEAMVR_SUPPORTED_PLATFORMS
FSteamVRHMD* GetSteamVRHMD()
{
	if (GEngine->XRSystem.IsValid() && (GEngine->XRSystem->GetSystemName() == FSteamVRHMD::SteamSystemName))
	{
		return static_cast<FSteamVRHMD*>(GEngine->XRSystem.Get());
	}

	return nullptr;
}

IMotionController* GetSteamMotionController()
{
	static FName DeviceTypeName(TEXT("SteamVRController"));
	TArray<IMotionController*> MotionControllers = IModularFeatures::Get().GetModularFeatureImplementations<IMotionController>(IMotionController::GetModularFeatureName());
	for (IMotionController* MotionController : MotionControllers)
	{
		if (MotionController->GetMotionControllerDeviceTypeName() == DeviceTypeName)
		{
			return MotionController;
		}
	}
	return nullptr;
}
#endif // STEAMVR_SUPPORTED_PLATFORMS

void USteamVRFunctionLibrary::GetValidTrackedDeviceIds(ESteamVRTrackedDeviceType DeviceType, TArray<int32>& OutTrackedDeviceIds)
{
#if STEAMVR_SUPPORTED_PLATFORMS
	OutTrackedDeviceIds.Empty();

	EXRTrackedDeviceType XRDeviceType = EXRTrackedDeviceType::Invalid;
	switch (DeviceType)
	{
	case ESteamVRTrackedDeviceType::Controller:
		XRDeviceType = EXRTrackedDeviceType::Controller;
		break;
	case ESteamVRTrackedDeviceType::TrackingReference:
		XRDeviceType = EXRTrackedDeviceType::TrackingReference;
		break;
	case ESteamVRTrackedDeviceType::Other:
		XRDeviceType = EXRTrackedDeviceType::Other;
		break;
	case ESteamVRTrackedDeviceType::Invalid:
		XRDeviceType = EXRTrackedDeviceType::Invalid;
		break;
	default:
		break;
	}


	FSteamVRHMD* SteamVRHMD = GetSteamVRHMD();
	if (SteamVRHMD)
	{
		SteamVRHMD->EnumerateTrackedDevices(OutTrackedDeviceIds, XRDeviceType);
	}
#endif // STEAMVR_SUPPORTED_PLATFORMS
}

bool USteamVRFunctionLibrary::GetTrackedDevicePositionAndOrientation(int32 DeviceId, FVector& OutPosition, FRotator& OutOrientation)
{
	bool RetVal = false;

#if STEAMVR_SUPPORTED_PLATFORMS
	FSteamVRHMD* SteamVRHMD = GetSteamVRHMD();
	if (SteamVRHMD)
	{
		FQuat DeviceOrientation = FQuat::Identity;
		RetVal = SteamVRHMD->GetCurrentPose(DeviceId, DeviceOrientation, OutPosition);
		OutOrientation = DeviceOrientation.Rotator();
	}
#endif // STEAMVR_SUPPORTED_PLATFORMS

	return RetVal;
}

bool USteamVRFunctionLibrary::GetHandPositionAndOrientation(int32 ControllerIndex, EControllerHand Hand, FVector& OutPosition, FRotator& OutOrientation)
{
	bool RetVal = false;

#if STEAMVR_SUPPORTED_PLATFORMS
	IMotionController* SteamMotionController = GetSteamMotionController();
	if (SteamMotionController)
	{
		// Note: the steam motion controller ignores the WorldToMeters scale argument.
		RetVal = static_cast<FXRMotionControllerBase*>(SteamMotionController)->GetControllerOrientationAndPosition(ControllerIndex, Hand, OutOrientation, OutPosition, -1.0f);
	}
#endif // STEAMVR_SUPPORTED_PLATFORMS

	return RetVal;
}

// Adam Rehberg - Engine Mod - Vive on SteamVR
void USteamVRFunctionLibrary::SetCustomSteamSplashScreenData(bool UseCustomData, FTransform CustomTransform, float CustomWorldScale)
{
#if STEAMVR_SUPPORTED_PLATFORMS
	FSteamVRHMD* SteamVRHMD = GetSteamVRHMD();
	if (SteamVRHMD)
	{
		SteamVRHMD->CustomSplashScreenData = UseCustomData;
		SteamVRHMD->CustomSplashScreenTransform = CustomTransform;
		SteamVRHMD->CustomSplashScreenWorldScale = CustomWorldScale;
	}
#endif // STEAMVR_SUPPORTED_PLATFORMS
}

// BEGIN - Engine Mod //

// Jacob Saueressig //
#if STEAMVR_SUPPORTED_PLATFORMS
// Wrapper around vr::IVRSystem::GetStringTrackedDeviceProperty //
// Copied directly from SteamVRHMD.cpp //
static FString GetFStringTrackedDevicePropertyLocal(vr::IVRSystem* VRSystem, uint32 DeviceIndex, vr::ETrackedDeviceProperty Property, vr::TrackedPropertyError* ErrorPtr = nullptr)
{
	check(VRSystem != nullptr);

	// Initialize //
	vr::TrackedPropertyError Error = vr::TrackedPropertyError::TrackedProp_Success;
	TArray<char> Buffer;
	Buffer.AddUninitialized(vr::k_unMaxPropertyStringSize);

	int Size = VRSystem->GetStringTrackedDeviceProperty(DeviceIndex, Property, Buffer.GetData(), Buffer.Num(), &Error);
	if (Error == vr::TrackedProp_BufferTooSmall)
	{
		Buffer.AddUninitialized(Size - Buffer.Num());
		Size = VRSystem->GetStringTrackedDeviceProperty(DeviceIndex, Property, Buffer.GetData(), Buffer.Num(), &Error);
	}

	if (ErrorPtr)
	{
		*ErrorPtr = Error;
		// Untracked Device? //
		if (Size <= 0)
			*ErrorPtr = vr::TrackedProp_InvalidDevice;
	}

	if (Error == vr::TrackedProp_Success)
		return UTF8_TO_TCHAR(Buffer.GetData());
	else
		return UTF8_TO_TCHAR(VRSystem->GetPropErrorNameFromEnum(Error));
}
#endif // STEAMVR_SUPPORTED_PLATFORMS

FString USteamVRFunctionLibrary::GetFStringControllerModel()
{
	FString Model("NULL");
#if STEAMVR_SUPPORTED_PLATFORMS
	FSteamVRHMD* SteamVRHMD = GetSteamVRHMD();
	if (SteamVRHMD == nullptr)
		return Model;

	vr::IVRSystem* VRSystem = SteamVRHMD->GetVRSystem();
	if (VRSystem == nullptr)
		return Model;

	for (uint32 DeviceIndex = 0; DeviceIndex < 8; ++DeviceIndex)
	{
		vr::TrackedPropertyError ErrorPtr = vr::TrackedPropertyError::TrackedProp_Success;
		Model = GetFStringTrackedDevicePropertyLocal(VRSystem, DeviceIndex, vr::Prop_ModelNumber_String, &ErrorPtr); // Prop_ModelNumber_String, Prop_RenderModelName_String

		// Device is tracked without error? //
		if (ErrorPtr == vr::TrackedPropertyError::TrackedProp_Success)
		{
			// Uncomment UE_LOG print statement for all valid device printouts //
			/*UE_LOG(LogTemp, Warning, TEXT("[SteamVRFunctionLibrary - GetFStringControllerModel] *OpenVR* DeviceIndex: [%i] - Model: [%s] - ErrorPtr: [%i]"),
											DeviceIndex,
											*Model,
											ErrorPtr);*/
			if(Model.Contains("Knuckles") || Model.Contains("Controller"))
				return Model;
		}
	}
#endif // STEAMVR_SUPPORTED_PLATFORMS
	return Model;
}

// END - Engine Mod //