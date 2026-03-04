/**
 * HMD Position Test Shader
 * 
 * This shader demonstrates how to read HMD positional data from OpenXR/SteamVR
 * and visualize it by mapping XYZ position to RGB colors.
 * 
 * Works in both VR and non-VR games - as long as a VR runtime (OpenXR or SteamVR) is running.
 */

// HMD center position as a float3 (X, Y, Z in meters)
uniform float3 HMDPosition < source = "hmd_position"; >;

// HMD center rotation as a float4 quaternion (X, Y, Z, W)
uniform float4 HMDRotation < source = "hmd_rotation"; >;

// HMD left eye position
uniform float3 HMDPositionLeft < source = "hmd_position_left"; >;

// HMD right eye position
uniform float3 HMDPositionRight < source = "hmd_position_right"; >;

// Vertex shader
void VS_HMDPosition(
	in uint id : SV_VertexID,
	out float4 position : SV_Position,
	out float2 texcoord : TEXCOORD0)
{
	// Generate a full-screen triangle
	texcoord.x = (id == 2) ? 2.0 : 0.0;
	texcoord.y = (id == 1) ? 2.0 : 0.0;
	position = float4(texcoord * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
}

// Pixel shader - map HMD position to RGB
float4 PS_HMDPosition(
	float4 position : SV_Position,
	float2 texcoord : TEXCOORD0) : SV_Target
{
	// Map X, Y, Z position to R, G, B colors
	// We normalize the position by dividing by a scale factor
	// Typical play space is about 2-3 meters, so we'll use that range
	float scale = 2.0;
	
	float3 color;
	color.r = (HMDPosition.x / scale) * 0.5 + 0.5; // X position to Red (normalized to 0-1)
	color.g = (HMDPosition.y / scale) * 0.5 + 0.5; // Y position to Green (normalized to 0-1)
	color.b = (HMDPosition.z / scale) * 0.5 + 0.5; // Z position to Blue (normalized to 0-1)
	
	// Clamp to valid color range
	color = saturate(color);
	
	return float4(color, 1.0);
}

// Pixel shader - visualize rotation as color
float4 PS_HMDRotation(
	float4 position : SV_Position,
	float2 texcoord : TEXCOORD0) : SV_Target
{
	// Convert quaternion to Euler angles for visualization
	// This is a simplified visualization - just showing the quaternion components
	float3 color;
	color.r = HMDRotation.x * 0.5 + 0.5; // X rotation component
	color.g = HMDRotation.y * 0.5 + 0.5; // Y rotation component
	color.b = HMDRotation.z * 0.5 + 0.5; // Z rotation component
	
	return float4(saturate(color), 1.0);
}

// Pixel shader - show eye separation
float4 PS_EyeSeparation(
	float4 position : SV_Position,
	float2 texcoord : TEXCOORD0) : SV_Target
{
	// Calculate distance between left and right eye positions
	float3 eye_diff = HMDPositionRight - HMDPositionLeft;
	float eye_distance = length(eye_diff);
	
	// Typical IPD (Inter-Pupillary Distance) is around 0.063 meters (63mm)
	// Map this to a color gradient
	float normalized_distance = eye_distance / 0.1; // 0-10cm range
	
	float3 color = float3(normalized_distance, 1.0 - normalized_distance, 0.5);
	
	return float4(saturate(color), 1.0);
}

// Techniques
technique HMDPositionVisualization
{
	pass
	{
		VertexShader = VS_HMDPosition;
		PixelShader = PS_HMDPosition;
	}
}

technique HMDRotationVisualization
{
	pass
	{
		VertexShader = VS_HMDPosition;
		PixelShader = PS_HMDRotation;
	}
}

technique HMDEyeSeparationVisualization
{
	pass
	{
		VertexShader = VS_HMDPosition;
		PixelShader = PS_EyeSeparation;
	}
}
