# To Do List

This document tracks the progress of porting Doom engines to the Motorola P2K platform.

*Updated: 30-Jul-2025*

## Legend:
- ✅ Done.
- 🟩 Planned.
- 🟨 In progress.
- 🟥 Blocked / Needs Fixing.

## 1. [P2kDoom8](../P2kDoom8) (Doom8088 port from old 16-bit DOS computers)
- ✅ Separated step and loop logic.
- ✅ Ported to SDL.
- ✅ Verified Big-Endian compatibility.
- ✅ Ported to P2K (supports ATI, Nvidia, DAL blitters).
- ✅ Fixed Melt/Wipe screen effect on P2K.
- ✅ Verified resource rebalancer (WAD filesystem reading).
- ✅ Ported and tested on Neptune.
- ✅ Ported and tested on Rainbow.
- 🟩 Ported and tested on Argon.
- 🟨 Ported to Motorola C650 and similar low-performance phones.
- 🟨 Implemented IRAM support on Neptune.
- 🟨 Final polishing, refactoring, GitHub release, and binary builds.

## 2. [P2kDoomG](../P2kDoomG) (GBADoom port from Nintendo Game Boy Advance)
- ✅ Separated step and loop logic.
- ✅ Ported to SDL.
- ✅ Ported to Big-Endian.
- ✅ Ported to P2K (supports ATI, Nvidia, DAL blitters).
- ✅ Fixed Melt/Wipe screen effect on P2K.
- 🟥 Implemented resource rebalancer (WAD filesystem reading) (needs rewriting).
- ✅ Ported and tested on Neptune.
- ✅ Ported and tested on Rainbow.
- ✅ Ported and tested on Argon.
- 🟨 Implemented IRAM support on Neptune.
- 🟩 Final polishing, refactoring, GitHub release, and binary builds.
- 🟩 Migrated to the GBADoomForDOS codebase (by FrenkelS).

## 3. [P2kDoomI](../P2kDoomI) (iDoom port from classic Apple iPods)
- 🟩 Separated step and loop logic.
- ✅ Ported to SDL.
- 🟥 Ported to Big-Endian (requires WAD repacker development).
- 🟩 Ported to P2K (supports ATI, Nvidia, DAL blitters).
- 🟩 Fixed Melt/Wipe screen effect on P2K.
- ✅ Verified resource rebalancer (WAD filesystem reading).
- 🟩 Ported and tested on Rainbow.
- 🟩 Ported and tested on Argon.
- 🟩 Final polishing, refactoring, GitHub release, and binary builds.
