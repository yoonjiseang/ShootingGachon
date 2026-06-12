# Submission Criteria for "C++ Conversion Complete"

This document defines the practical threshold for saying the project has been converted from Blueprint gameplay logic to C++.

## Recommended professor-facing standard

It is reasonable to say **"C++ conversion complete"** when the following are true:

1. `ShootingMap_Wave` or an equivalent converted map runs successfully.
2. Core gameplay logic is handled by native C++ classes:
   - wave start / next wave
   - normal enemy spawn
   - boss spawn
   - score updates
   - player HP damage
   - game over / restart
   - power-up effects and timers
3. The original Blueprint Event Graphs are no longer the main source of gameplay rules.
4. Blueprint assets, if they still exist, act mainly as:
   - presentation wrappers
   - component/layout holders
   - asset reference containers
5. Rebuilding the project and reopening the editor does not break the converted path.

## What does NOT have to be true

These do **not** need to be true to claim a realistic Unreal-style C++ conversion:

- every Blueprint file deleted from the project
- every UI widget converted to native-only rendering
- every mesh/material/sound reference moved into code

In Unreal projects, it is normal to keep Blueprint assets for:
- visuals
- component placement
- material assignment
- sound/particle hookups
- UI layout

## Current status after the staged conversion step

The project currently satisfies these parts:

- C++ module exists and builds successfully
- native gameplay classes were implemented
- staged C++ conversion assets were generated under `/Game/CppConverted`
- important defaults were verified through Unreal Python inspection
- original assets were preserved

The project does **not** yet fully satisfy these parts:

- the main playable map has not been fully swapped to the staged converted assets
- the original player movement/input behavior has not been fully replaced in native code
- a complete end-to-end "no original gameplay Blueprint dependency" playtest has not been confirmed

## Honest status wording

The safest accurate wording right now is:

`C++ conversion framework and staged gameplay asset path completed. Full final asset replacement and player parity verification still remain.`

Do **not** overstate the current state as fully finished if:
- the original map still relies on original gameplay Blueprints for player control
- the staged native player has not yet been validated against the original control scheme

## Final completion checklist

Use this checklist before calling the conversion complete:

- `InteriorEditor` build succeeds
- staged C++ assets exist under `/Game/CppConverted`
- converted wave manager is used in a test map or duplicated map
- converted enemy/boss/bullet/power-up flow works in play
- HUD updates still work through the converted path
- game over and restart still work
- player movement/fire behavior matches the original required assignment behavior

## Recommended next implementation target

To move from "staged conversion" to "complete conversion", the next highest-value task is:

`Replace or reproduce the original player movement/input behavior in native C++ and validate it inside a converted test map.`
