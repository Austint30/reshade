/*
 * Copyright (C) 2024 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "runtime_vr.hpp"
#include <cstring>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Forward declarations for OpenXR
typedef struct XrInstance_T *XrInstance;
typedef struct XrSession_T *XrSession;
typedef int64_t XrTime;
typedef uint64_t XrSystemId;
typedef uint32_t XrStructureType;
typedef uint32_t XrResult;

struct XrVector3f { float x, y, z; };
struct XrQuaternionf { float x, y, z, w; };
struct XrPosef { XrQuaternionf orientation; XrVector3f position; };

// Forward declarations for OpenVR
namespace vr
{
	typedef uint32_t TrackedDeviceIndex_t;
	struct HmdMatrix34_t { float m[3][4]; };
	struct TrackedDevicePose_t
	{
		HmdMatrix34_t mDeviceToAbsoluteTracking;
		XrVector3f vVelocity;
		XrVector3f vAngularVelocity;
		int32_t eTrackingResult;
		bool bPoseIsValid;
		bool bDeviceIsConnected;
	};
	class IVRSystem;
}

// Helper to convert OpenVR matrix to position and quaternion
static void matrix_to_pose(const vr::HmdMatrix34_t &mat, reshade::hmd_pose &pose)
{
	// Extract position from the matrix
	pose.position[0] = mat.m[0][3];
	pose.position[1] = mat.m[1][3];
	pose.position[2] = mat.m[2][3];

	// Convert rotation matrix to quaternion
	float trace = mat.m[0][0] + mat.m[1][1] + mat.m[2][2];
	if (trace > 0.0f)
	{
		float s = 0.5f / sqrtf(trace + 1.0f);
		pose.rotation[3] = 0.25f / s;  // w
		pose.rotation[0] = (mat.m[2][1] - mat.m[1][2]) * s;  // x
		pose.rotation[1] = (mat.m[0][2] - mat.m[2][0]) * s;  // y
		pose.rotation[2] = (mat.m[1][0] - mat.m[0][1]) * s;  // z
	}
	else if (mat.m[0][0] > mat.m[1][1] && mat.m[0][0] > mat.m[2][2])
	{
		float s = 2.0f * sqrtf(1.0f + mat.m[0][0] - mat.m[1][1] - mat.m[2][2]);
		pose.rotation[3] = (mat.m[2][1] - mat.m[1][2]) / s;  // w
		pose.rotation[0] = 0.25f * s;  // x
		pose.rotation[1] = (mat.m[0][1] + mat.m[1][0]) / s;  // y
		pose.rotation[2] = (mat.m[0][2] + mat.m[2][0]) / s;  // z
	}
	else if (mat.m[1][1] > mat.m[2][2])
	{
		float s = 2.0f * sqrtf(1.0f + mat.m[1][1] - mat.m[0][0] - mat.m[2][2]);
		pose.rotation[3] = (mat.m[0][2] - mat.m[2][0]) / s;  // w
		pose.rotation[0] = (mat.m[0][1] + mat.m[1][0]) / s;  // x
		pose.rotation[1] = 0.25f * s;  // y
		pose.rotation[2] = (mat.m[1][2] + mat.m[2][1]) / s;  // z
	}
	else
	{
		float s = 2.0f * sqrtf(1.0f + mat.m[2][2] - mat.m[0][0] - mat.m[1][1]);
		pose.rotation[3] = (mat.m[1][0] - mat.m[0][1]) / s;  // w
		pose.rotation[0] = (mat.m[0][2] + mat.m[2][0]) / s;  // x
		pose.rotation[1] = (mat.m[1][2] + mat.m[2][1]) / s;  // y
		pose.rotation[2] = 0.25f * s;  // z
	}
}

// Try to query OpenVR for HMD pose
static bool query_openvr_hmd_pose(reshade::hmd_pose &center_pose, reshade::hmd_pose &left_eye_pose, reshade::hmd_pose &right_eye_pose)
{
	// Try to load OpenVR library
	HMODULE openvr_module = GetModuleHandleW(L"openvr_api.dll");
	if (openvr_module == nullptr)
		return false;

	// Get the VR_GetGenericInterface function
	typedef void *(*PFN_VR_GetGenericInterface)(const char *, int *);
	const auto VR_GetGenericInterface = reinterpret_cast<PFN_VR_GetGenericInterface>(
		GetProcAddress(openvr_module, "VR_GetGenericInterface"));
	if (VR_GetGenericInterface == nullptr)
		return false;

	// Get IVRSystem interface
	int error = 0;
	vr::IVRSystem *vr_system = static_cast<vr::IVRSystem *>(
		VR_GetGenericInterface("IVRSystem_022", &error));
	if (vr_system == nullptr || error != 0)
		return false;

	// Query HMD pose (device index 0 is always the HMD)
	vr::TrackedDevicePose_t poses[1];
	typedef void (*PFN_GetDeviceToAbsoluteTrackingPose)(vr::IVRSystem *, int, float, vr::TrackedDevicePose_t *, uint32_t);
	
	// Get the virtual function for GetDeviceToAbsoluteTrackingPose (vtable index 11)
	void **vtable = *reinterpret_cast<void ***>(vr_system);
	const auto GetDeviceToAbsoluteTrackingPose = reinterpret_cast<PFN_GetDeviceToAbsoluteTrackingPose>(vtable[11]);
	
	// ETrackingUniverseOrigin::TrackingUniverseStanding = 1
	GetDeviceToAbsoluteTrackingPose(vr_system, 1, 0.0f, poses, 1);

	if (!poses[0].bPoseIsValid || !poses[0].bDeviceIsConnected)
		return false;

	// Convert matrix to pose
	matrix_to_pose(poses[0].mDeviceToAbsoluteTracking, center_pose);

	// Get eye transforms relative to HMD
	typedef vr::HmdMatrix34_t (*PFN_GetEyeToHeadTransform)(vr::IVRSystem *, int);
	const auto GetEyeToHeadTransform = reinterpret_cast<PFN_GetEyeToHeadTransform>(vtable[13]);
	
	// Get left eye (0) and right eye (1) transforms
	vr::HmdMatrix34_t left_transform = GetEyeToHeadTransform(vr_system, 0);
	vr::HmdMatrix34_t right_transform = GetEyeToHeadTransform(vr_system, 1);
	
	// Convert eye transforms to poses (these are relative to HMD)
	matrix_to_pose(left_transform, left_eye_pose);
	matrix_to_pose(right_transform, right_eye_pose);
	
	// Combine HMD pose with eye transforms for absolute eye positions
	// For simplicity, we'll just use the relative positions for now
	// In a full implementation, you'd transform these by the HMD pose
	
	return true;
}

// Try to query OpenXR for HMD pose
static bool query_openxr_hmd_pose(reshade::hmd_pose &center_pose, reshade::hmd_pose &left_eye_pose, reshade::hmd_pose &right_eye_pose)
{
	// OpenXR is more complex to query without an active session
	// For non-VR games, we'd need to create our own session
	// This is a placeholder for now
	return false;
}

#endif

bool reshade::query_vr_runtime_hmd_pose(hmd_pose &center_pose, hmd_pose &left_eye_pose, hmd_pose &right_eye_pose)
{
#if defined(_WIN32)
	// Try OpenVR first (more commonly available)
	if (query_openvr_hmd_pose(center_pose, left_eye_pose, right_eye_pose))
		return true;

	// Try OpenXR as fallback
	if (query_openxr_hmd_pose(center_pose, left_eye_pose, right_eye_pose))
		return true;
#endif

	// No VR runtime available
	return false;
}
