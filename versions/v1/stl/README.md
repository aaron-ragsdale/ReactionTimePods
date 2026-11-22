# Version 1 — 3D printed parts

This folder contains the printable STL files for the **four-button, wired OpenSportSense Reaction Pods Version 1**. The [editable Autodesk Fusion design](reaction_pods.f3d) is also included for anyone who wants to change dimensions, redesign the components, or export modified STL files.

## What to print

Print **one of every model per button, except the spring, which needs four copies per button**.

| Model | Per button | Complete 4-button set | Purpose |
| --- | ---: | ---: | --- |
| [Base.stl](Base.stl) | 1 | 4 | Main pod base |
| [Base_cover.stl](Base_cover.stl) | 1 | 4 | Base cover |
| [Button_base.stl](Button_base.stl) | 1 | 4 | Button support / mounting part |
| [Button_top.stl](Button_top.stl) | 1 | 4 | Pressable top |
| [Spring.stl](Spring.stl) | **4** | **16** | Printed spring elements for button return |
| [pressfit_leg_6mm.stl](pressfit_leg_6mm.stl) | 2-4 | 8-16 | Press-fit support for a 6 mm stand material |
| **Total** | **9** | **36** | Printed parts for four buttons |

The press-fit leg is designed around **6 mm material** (for example, plywood). You can substitute another rigid stand material, but check fit or edit the Fusion design to match its thickness.

## Recommended printers

A reliable **FDM/FFF printer** with a 0.4 mm nozzle and an adequately sized bed should work for these parts. Suitable examples include Bambu Lab A1/A1 mini/P1S, Prusa MK4-series, and comparable well-calibrated Creality printers. These are **printer examples, not a claim that each model has been test-printed on every machine**. Confirm that your chosen printer's bed fits the largest model in your slicer.

Resin printing is not the default recommendation, particularly for parts that must flex.

## Suggested starting print settings

| Setting | Suggested starting point |
| --- | --- |
| Process | FDM / FFF |
| Nozzle | 0.4 mm |
| Layer height | 0.20 mm (0.16 mm for smoother visible surfaces if desired) |
| Material | PLA for easy prototyping; PETG for tougher parts and as an option to test on the printed springs |
| Perimeters / walls | 3–4 |
| Infill | 20–30% for the larger rigid components |
| Top/bottom layers | 4–5 |
| Supports | Avoid where possible; enable only after checking overhangs in slicer preview |
| Brim | Optional for small, narrow, or lifting parts |
| Scale | 100% initially; verify fit before changing dimensions |

**Spring note:** The spring pieces are functional flexing components. Try PETG for greater toughness and print a **single spring first** to test flexibility and button return before committing to all 16. Spring performance depends on geometry, layer bonding, wall count, and print direction, so the general infill setting above is not automatically optimal for springs. Do not assume TPU or other flexible filaments are interchangeable with the designed spring without testing.

These are **starting recommendations**, not validated print profiles for the supplied files.

## Orientation tips

- **Base.stl:** Start with its broadest stable flat surface on the build plate. Check that mounting points and openings are not filled by automatic supports.
- **Base_cover.stl:** Place its broad, flat face on the build plate when possible; inspect any clips or attachment details.
- **Button_base.stl:** Orient the stable mounting face downward where possible. Check openings, screw holes, and the button seat in the slicer preview.
- **Button_top.stl:** Choose the orientation that gives the visible pressing surface the finish you want while minimizing supports on fit-critical surfaces. Inspect the inside of the cap for overhangs.
- **Spring.stl:** **Do not simply use the largest-face-down rule.** Preview the layers and orient the spring so repeated flexing does not tend to split weak interlayer bonds. Print one, test the motion, and adjust orientation before printing the rest.
- **pressfit_leg_6mm.stl:** Start on its most stable flat face and check the press-fit opening and contact surfaces. A brim may help if the footprint is narrow.

Exact support needs and optimal orientations should be confirmed after checking the actual model geometry in your slicer. Avoid supports on press-fit and sliding surfaces unless necessary.

## Print and assembly tips

1. Print **one complete button's parts first**, including **four springs**, to confirm fit and motion.
2. Before printing a set of four, test the 12 × 12 × 7.3 mm tactile switch fit, the spring action, cap travel, and LED clearance.
3. Test the 6 mm stand material against the press-fit leg. Nominal 6 mm plywood may vary in actual thickness.
4. Clean up stringing or elephant's-foot expansion around mating edges before forcing parts together.
5. If printed parts bind, check printer calibration and fit tolerances before scaling the whole part.
6. If using heat-set inserts, confirm the actual insert and screw dimensions match the modeled holes. Do not force hot inserts into undersized walls.
7. Keep some spare springs: they are small, flexing parts and may wear or require tuning with different filaments.

## Edit the models in Autodesk Fusion

The file [**reaction_pods.f3d**](reaction_pods.f3d) contains the Fusion design source.

To customize it:

1. Download the `.f3d` file.
2. Open Autodesk Fusion and import/open the design.
3. Modify the geometry for your switches, LEDs, stand material, or enclosure preferences.
4. Export the modified components as STL or 3MF for slicing.
5. If sharing a modified build, include the updated source file and note any hardware changes needed.

The STL files are ready-to-slice geometry; the Fusion file is the better starting point for dimensional changes.

## Next steps

See the [Version 1 overview](../README.md) and [build guide](../BUILD_GUIDE.md) for assembly, electronics, firmware, and testing. More detailed photos and verified per-part print profiles can be added as the design evolves.
