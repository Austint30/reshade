# Virtual Desktop Compatibility - OpenXR Session Conflict Prevention

## Issue

Creating OpenXR sessions from ReShade for non-VR games could interfere with existing VR applications, specifically:

- **Virtual Desktop Classic** - Allows viewing non-VR games in VR
- **SteamVR Overlays** - Desktop view and other overlay applications
- **Other VR Applications** - Any app with an active OpenXR session

Multiple OpenXR sessions can cause:
- Runtime instability
- Session conflicts
- Application crashes
- Loss of VR tracking

## Solution

ReShade uses **OpenVR (SteamVR) queries only** for non-VR games. This approach:

✅ **Safe:** Reads from existing runtime state without creating new sessions
✅ **Compatible:** Works with Virtual Desktop and all VR viewing apps
✅ **Reliable:** No risk of session conflicts or interference
✅ **Simple:** Direct queries without complex session management

## Implementation Details

### What ReShade Does

For **non-VR games**:
1. Dynamically loads `openvr_api.dll`
2. Gets IVRSystem interface (tries multiple versions for compatibility)
3. Queries HMD tracking state via `GetDeviceToAbsoluteTrackingPose()`
4. Reads eye transforms via `GetEyeToHeadTransform()`
5. Returns pose data to shaders

For **VR games**:
- Could extract pose data from existing OpenXR/OpenVR sessions in game hooks
- No additional queries needed

### What ReShade Does NOT Do

❌ Does NOT create OpenXR instances
❌ Does NOT create OpenXR sessions
❌ Does NOT initialize new VR runtimes
❌ Does NOT interfere with existing VR applications

## Code Evidence

From `runtime_vr.cpp`:

```cpp
// Try to query OpenXR for HMD pose
// 
// WARNING: DO NOT IMPLEMENT OpenXR SESSION CREATION FOR NON-VR GAMES
// Creating an OpenXR session while another VR application (e.g., SteamVR, Virtual Desktop)
// is running can cause conflicts and interfere with the existing VR session. This could
// break features like viewing games through Virtual Desktop Classic or other VR overlays.
//
static bool query_openxr_hmd_pose(...)
{
    // INTENTIONALLY NOT IMPLEMENTED to avoid interfering with existing VR sessions
    return false;
}
```

The main query function only uses OpenVR:

```cpp
bool reshade::query_vr_runtime_hmd_pose(...)
{
    // Try OpenVR (SteamVR) - safe to query without creating new sessions
    if (query_openvr_hmd_pose(...))
        return true;

    // Note: OpenXR query is intentionally not implemented for non-VR games
    // Creating OpenXR sessions would interfere with existing VR applications
    return false;
}
```

## Testing Scenarios

### Scenario 1: Virtual Desktop Classic
**Setup:**
- SteamVR running
- Virtual Desktop viewing non-VR game
- ReShade with HMD tracking enabled

**Result:** ✅ Works correctly
- Virtual Desktop continues functioning
- ReShade receives HMD pose data via OpenVR
- No session conflicts
- No interference

### Scenario 2: SteamVR Overlay
**Setup:**
- SteamVR running
- Desktop overlay or other VR app active
- ReShade in non-VR game

**Result:** ✅ Works correctly
- Overlay remains functional
- ReShade queries OpenVR successfully
- No conflicts

### Scenario 3: No VR Runtime
**Setup:**
- SteamVR not running
- ReShade in non-VR game

**Result:** ✅ Graceful degradation
- ReShade returns default values (zeros)
- No errors or crashes
- Game continues normally

## Alternative Approaches Considered

### ❌ Option 1: Create OpenXR Session
**Problem:** Would conflict with Virtual Desktop and other VR apps
**Rejected**

### ❌ Option 2: Check for Existing Session First
**Problem:** Still creates session if none exists, could interfere later
**Rejected**

### ✅ Option 3: Use OpenVR Only (Current Implementation)
**Advantages:**
- No session conflicts
- Compatible with all VR viewing apps
- Simple and reliable
- Covers vast majority of users (SteamVR is dominant)

**Selected**

## Compatibility Matrix

| Scenario | OpenVR Query | OpenXR Session | Result |
|----------|--------------|----------------|--------|
| Non-VR game + Virtual Desktop | ✅ Yes | ❌ No | ✅ Compatible |
| Non-VR game + SteamVR overlay | ✅ Yes | ❌ No | ✅ Compatible |
| VR game (OpenXR) | ⚪ N/A | ⚪ Use existing | ✅ Compatible |
| VR game (OpenVR) | ⚪ N/A | ⚪ Use existing | ✅ Compatible |
| No VR runtime | ⚪ Returns false | ❌ No | ✅ Safe |

## Future Considerations

If OpenXR support is needed in the future:

1. **Only for VR games:** Extract from existing session, never create new ones
2. **Detection:** Check if OpenXR session already exists before any action
3. **Fallback:** Always prefer OpenVR for non-VR games
4. **Documentation:** Maintain clear warnings about conflicts

## Conclusion

The current implementation is **safe and compatible** with Virtual Desktop Classic and all other VR viewing applications. It achieves the goal of providing HMD tracking data to shaders without creating any risk of session conflicts or interference.

**Recommendation:** Keep the current OpenVR-only approach for non-VR games. Do not implement OpenXR session creation.
