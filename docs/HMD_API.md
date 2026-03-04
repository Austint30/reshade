# HMD Positional Data API Documentation

## Overview

ReShade now supports reading HMD (Head-Mounted Display) position and rotation data from OpenXR and SteamVR runtimes. This data can be accessed in shaders through special uniform variables and works in **both VR games and non-VR games**, as long as a VR runtime is running on the system.

## Shader API

### Special Uniform Annotations

To access HMD data in your ReShade shaders, declare uniform variables with the `source` annotation:

```hlsl
// HMD center position in world space (meters)
uniform float3 HMDPosition < source = "hmd_position"; >;

// HMD center rotation as quaternion (x, y, z, w)
uniform float4 HMDRotation < source = "hmd_rotation"; >;

// Per-eye data
uniform float3 HMDPositionLeft < source = "hmd_position_left"; >;
uniform float4 HMDRotationLeft < source = "hmd_rotation_left"; >;
uniform float3 HMDPositionRight < source = "hmd_position_right"; >;
uniform float4 HMDRotationRight < source = "hmd_rotation_right"; >;
```

### Available Sources

| Source Name | Type | Description |
|------------|------|-------------|
| `hmd_position` | `float3` | HMD center position (X, Y, Z) in meters |
| `hmd_rotation` | `float4` | HMD center rotation quaternion (x, y, z, w) |
| `hmd_position_left` | `float3` | Left eye position in meters |
| `hmd_rotation_left` | `float4` | Left eye rotation quaternion |
| `hmd_position_right` | `float3` | Right eye position in meters |
| `hmd_rotation_right` | `float4` | Right eye rotation quaternion |

## Example Shader

See `examples/18-hmd_position_shader/HMDPositionTest.fx` for a complete working example that visualizes HMD position and rotation data.

## Use Cases

- **Head-tracking effects**: Create parallax, depth-based, or motion effects
- **VR debugging**: Visualize tracking quality and play space boundaries
- **Interactive shaders**: Respond to user head movement
- **Presence detection**: Detect if HMD is being worn
- **IPD calculation**: Calculate inter-pupillary distance

## Technical Details

- **Performance**: Minimal overhead - one query per frame
- **Default values**: (0,0,0) position, (0,0,0,1) rotation when VR runtime unavailable
- **Coordinate system**: Matches VR runtime (typically standing/room-scale)
- **Units**: Position in meters, rotation as normalized quaternion

## Requirements

- OpenXR or SteamVR runtime must be running
- HMD must be powered on and tracking
- Works in both VR and non-VR games

## See Also

- Example shader: `examples/18-hmd_position_shader/`
- ReShade reference: `REFERENCE.md`
