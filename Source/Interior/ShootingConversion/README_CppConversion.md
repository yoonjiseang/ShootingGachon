# ShootingConversion C++ Guide

This folder contains the staged C++ conversion for the original Blueprint-based shooting game.

The project now has:
- a valid Unreal C++ module
- native gameplay classes for wave/enemy/boss/bullet/power-up logic
- a safe parallel asset path under `/Game/CppConverted`

The original Blueprint assets were intentionally left in place to avoid damaging the working game.

## Native classes

Core native classes live in `Source/Interior/ShootingConversion/`.

- `AShootingWaveManager`
- `AShootingEnemy`
- `AShootingBossEnemy`
- `AShootingBullet`
- `AShootingBossBullet`
- `AShootingPowerUpOrb`
- `AShootingPlayer`
- `UShootingGameHUDWidget`
- `EShootingPowerUpType`

## Parallel conversion assets

Generated staged assets live in `/Game/CppConverted`.

- `BP_Bullet_Cpp`
- `BP_BossBullet_Cpp`
- `BP_Enemy_Cpp`
- `BP_BossEnemy_Cpp`
- `BP_PowerUpOrb_Cpp`
- `BP_Player_Cpp`
- `BP_WaveManager_Cpp`

These assets are designed to let the conversion move forward without reparenting the original `.uasset` files.

## Verified defaults

The staged assets were generated and verified through Unreal Python commandlets.

Verified examples:
- `BP_Bullet_Cpp`
  - `DamageAmount = 1.0`
  - `Speed = 1200.0`
- `BP_BossBullet_Cpp`
  - `DamageAmount = 1.0`
  - `Speed = 700.0`
- `BP_Enemy_Cpp`
  - `HP = 1.0`
  - `ScoreValue = 100`
  - `PowerUpOrbClass = /Game/CppConverted/BP_PowerUpOrb_Cpp`
- `BP_BossEnemy_Cpp`
  - `HP = 20.0`
  - `MaxHP = 20.0`
  - `ScoreValue = 1000`
  - `BossBulletClass = /Game/CppConverted/BP_BossBullet_Cpp`
- `BP_PowerUpOrb_Cpp`
  - `EffectType = INVINCIBLE`
  - `DamageMultiplier = 2.0`
  - `BuffDuration = 3.0`
  - `HealAmount = 1.0`
- `BP_Player_Cpp`
  - `BulletClass = /Game/CppConverted/BP_Bullet_Cpp`
  - `BaseBulletDamage = 1.0`
- `BP_WaveManager_Cpp`
  - `SpawnPointClass = /Game/Blueprints/BP_EnemyManager`
  - `EnemyClass = /Game/CppConverted/BP_Enemy_Cpp`
  - `BossEnemyClass = /Game/CppConverted/BP_BossEnemy_Cpp`
  - `HUDWidgetClass = /Game/Game/UI/WBP_GameHUD`
  - `RestartLevelName = ShootingMap_Wave`
  - `SpawnCooldown = 0.8`
  - `BossWaveInterval = 3`

## Bridge behavior

Some native classes were adjusted so they can coexist with the original Blueprint assets during migration.

Examples:
- `AShootingWaveManager` can call the original `WBP_GameHUD` update functions through reflection.
- `AShootingEnemy` and `AShootingBossEnemy` can read `DamageAmount` from any overlapping bullet actor that exposes that property.
- `AShootingBossBullet` can damage the current player pawn without requiring a native player cast.

This allows staged migration instead of an all-at-once replacement.

## Current status

What is done:
- project converted to an Unreal C++ project
- native gameplay classes implemented
- staged `/Game/CppConverted` assets generated
- build succeeds
- staged asset defaults verified

What is not fully done yet:
- the original map has not been fully swapped to the staged C++ assets
- the original player movement/input Blueprint behavior has not been completely replaced in native code
- the final Blueprint-free gameplay path is not fully validated yet

## Safe next step

Recommended next migration step:

1. keep the original map as a backup
2. create a separate test map or duplicate map
3. replace actors one by one with the staged `/Game/CppConverted` assets
4. test after every replacement

Recommended replacement order:

1. `BP_Bullet_Cpp`
2. `BP_BossBullet_Cpp`
3. `BP_Enemy_Cpp`
4. `BP_BossEnemy_Cpp`
5. `BP_PowerUpOrb_Cpp`
6. `BP_WaveManager_Cpp`
7. `BP_Player_Cpp` after player movement/input parity is confirmed

## Verification artifacts

Useful generated inspection files:

- `Tools/conversion_data_dump.txt`
- `Tools/debug_unreal_symbols_dump.txt`
- `Tools/cpp_converted_assets_dump.txt`

These files document the extracted defaults and staged asset state.
