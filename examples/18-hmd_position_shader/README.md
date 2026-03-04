# HMD Position Shader Example

This example demonstrates how to use ReShade to read HMD (Head-Mounted Display) positional data from SteamVR (OpenVR) and use it in custom shaders.

**Important:** This implementation uses **OpenVR (SteamVR)** only to avoid interfering with existing VR applications (e.g., Virtual Desktop, SteamVR overlays). Creating OpenXR sessions could conflict with active VR sessions.

## Overview

ReShade can now query HMD position and rotation data from a running SteamVR runtime and make it available to shaders through special uniform variables. This works in **both VR and non-VR games**, as long as SteamVR is active on the system.

## Shader API

### Special Uniform Sources

To access HMD data in your shaders, declare uniform variables with the `source` annotation:

```hlsl
// HMD center (average of both eyes) position in world space (meters)
uniform float3 HMDPosition < source = "hmd_position"; >;

// HMD center rotation as a quaternion (x, y, z, w)
uniform float4 HMDRotation < source = "hmd_rotation"; >;

// Left eye position in world space (meters)
uniform float3 HMDPositionLeft < source = "hmd_position_left"; >;

// Left eye rotation as a quaternion (x, y, z, w)
uniform float4 HMDRotationLeft < source = "hmd_rotation_left"; >;

// Right eye position in world space (meters)
uniform float3 HMDPositionRight < source = "hmd_position_right"; >;

// Right eye rotation as a quaternion (x, y, z, w)
uniform float4 HMDRotationRight < source = "hmd_rotation_right"; >;
```

### Data Format

- **Position**: `float3` vector in meters representing (X, Y, Z) coordinates in world space
  - X: Left (-) to Right (+)
  - Y: Down (-) to Up (+)
  - Z: Forward (-) to Backward (+)
  
- **Rotation**: `float4` quaternion representing orientation as (X, Y, Z, W)
  - Normalized quaternion values typically in range [-1, 1]
  - W component is usually positive for normal orientations

### Example Shader

The `HMDPositionTest.fx` shader provides three visualization techniques:

1. **HMDPositionVisualization**: Maps XYZ position to RGB colors
2. **HMDRotationVisualization**: Visualizes rotation quaternion components as colors
3. **HMDEyeSeparationVisualization**: Shows the distance between left and right eye positions (IPD)

## Usage

### For VR Games

In VR games, HMD data is automatically captured from the VR session. The shader will receive real-time position and rotation data.

### For Non-VR Games

1. Ensure SteamVR is running on your system
2. Put on your HMD or keep it powered on
3. Launch the non-VR game with ReShade injected
4. Enable the HMD position shader
5. The shader will query SteamVR and display HMD position/rotation data

**Note:** Only SteamVR (OpenVR) is queried for non-VR games. This avoids creating OpenXR sessions that could interfere with Virtual Desktop Classic or other VR viewing applications.

## Use Cases

- **Head tracking for camera control**: Use HMD position to control in-game camera
- **Interactive effects**: Create effects that respond to head movement
- **VR debugging**: Visualize HMD tracking quality and range
- **Motion-based shaders**: Create effects based on head velocity or rotation
- **Presence detection**: Detect if user is wearing the HMD

## Technical Details

- Position data is updated every frame before shader execution
- If no VR runtime is available, position defaults to (0, 0, 0) and rotation to identity quaternion (0, 0, 0, 1)
- **OpenVR (SteamVR)** is queried for non-VR games (safe, no session conflicts)
- **OpenXR** queries are not used for non-VR games to avoid interfering with existing VR applications
- For VR games using OpenXR, pose data could be extracted from existing game sessions
- Minimal performance impact - single query per frame

### Why Only SteamVR for Non-VR Games?

Creating a new OpenXR session for HMD queries could interfere with existing VR applications such as:
- **Virtual Desktop Classic** - viewing non-VR games in VR
- **SteamVR overlays** - desktop view and other VR tools
- **Other VR applications** - any app with an active session

By using OpenVR queries instead, ReShade safely reads from the existing SteamVR state without creating conflicting sessions.

## Limitations

- Requires SteamVR (OpenVR) runtime to be active for non-VR games
- Position/rotation is in the VR runtime's coordinate system
- May not be available if HMD is in standby mode
- Per-eye data represents the eye offsets from the HMD center position
- OpenXR sessions are not created to prevent interference with existing VR applications

## Building

This shader is included as an example and doesn't need to be built. Simply copy `HMDPositionTest.fx` to your ReShade shaders directory and enable it in the ReShade overlay.

## Related

- See ReShade documentation for other special uniform sources: `frametime`, `framecount`, `mousepoint`, etc.
- OpenXR specification: https://www.khronos.org/openxr/
- OpenVR (SteamVR) documentation: https://github.com/ValveSoftware/openvr
