/*
 * Copyright (C) 2024 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

namespace reshade
{
	/// <summary>
	/// Structure to hold HMD pose data (position and rotation)
	/// </summary>
	struct hmd_pose
	{
		float position[3] = { 0.0f, 0.0f, 0.0f };
		float rotation[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // quaternion (x, y, z, w)
	};

	/// <summary>
	/// Query HMD pose data from running OpenXR or SteamVR runtime
	/// Returns true if successful, false if no VR runtime is available
	/// </summary>
	bool query_vr_runtime_hmd_pose(hmd_pose &center_pose, hmd_pose &left_eye_pose, hmd_pose &right_eye_pose);
}

