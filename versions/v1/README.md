# Reaction Pods — Version 1

The first working OpenSportSense Reaction Pods prototype uses **one ESP32 controller and four wired, individually illuminated physical push-button targets**. The ESP32 drives the LEDs, reads each press, and hosts a local browser-based game interface.

**Status:** Working prototype. Hardware files are available; detailed assembly documentation is still in progress.

## Version 1 files

| Location | What it contains |
| --- | --- |
| [Firmware](firmware/) | ESP32 Arduino firmware, including [ReactionGame_ESP32_v1.ino](firmware/ReactionGame_ESP32_v1.ino) |
| [Interface](interface/) | Documentation/source area for the HTML dashboard served by the ESP32; the interface may be embedded directly in the firmware |
| [3D print files](stl/) | Six STL models and editable Autodesk Fusion file |
| [Build guide](BUILD_GUIDE.md) | Assembly, wiring, setup, and troubleshooting tutorial (in progress) |

## 3D-print checklist for four buttons

Each button uses **one of each STL part, plus four printed springs**.

| STL | Per button | Four buttons |
| --- | ---: | ---: |
| [Base](stl/Base.stl) | 1 | 4 |
| [Base cover](stl/Base_cover.stl) | 1 | 4 |
| [Button base](stl/Button_base.stl) | 1 | 4 |
| [Button top](stl/Button_top.stl) | 1 | 4 |
| [Spring](stl/Spring.stl) | **4** | **16** |
| [6 mm press-fit leg](stl/pressfit_leg_6mm.stl) | 1 | 4 |
| **Total** | **9** | **36** |

### Recommended printing approach

A well-calibrated FDM printer with a 0.4 mm nozzle is a good starting point. Examples include Bambu Lab A1/A1 mini/P1S, Prusa MK4-series, and comparable Creality machines, subject to build-volume compatibility.

Start with **0.2 mm layer height, 3–4 walls, 20–30% infill on rigid parts, and PLA or PETG**. Consider PETG for the springs, and print/test just one spring before printing 16. Put broad stable faces on the bed where practical, but choose spring orientation carefully to avoid layer splitting during flexing. The best settings and orientation depend on the specific geometry and your printer.

**Read the [full 3D printing instructions](stl/README.md) for per-part orientation advice, first-print fit checks, and additional spring tips.** These settings are suggestions, not verified print profiles for every printer.

### Customize the design

Want to change the size or shape of the pods? The [**reaction_pods.f3d Fusion file**](stl/reaction_pods.f3d) is included alongside the STLs. Open it in Autodesk Fusion, modify the components, and export your own printable models. Builders can adapt the design for different buttons, LEDs, mounting materials, and future games.

## Hardware and setup

See the [main project README](../../README.md) for an initial parts list and overview of how the ESP32, buttons, LEDs, and dashboard work.

The press-fit leg is intended for **6 mm stock**, such as plywood; another rigid material may be used if it fits. More exact wiring diagrams, assembly photos, power requirements, and firmware setup instructions will be added to the [build guide](BUILD_GUIDE.md).

Future revisions can be kept in their own version folders so this V1 design remains accessible.
