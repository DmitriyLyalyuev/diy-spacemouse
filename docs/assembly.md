# Assembly notes

## Printed parts

Print the mechanical parts from the CAD/STL files in `CAD/`. Check the latest available STLs before printing and keep the print orientation consistent between paired parts.

## Inserts and screws

The model was designed around M2.5 inserts and screws, but some insert holes in the tested prints were oversized. In those places, M2.5 inserts did not hold reliably and M3 inserts/screws were used instead.

Before final assembly:

- test-fit M2.5 inserts in every printed hole;
- switch to M3 where M2.5 inserts are loose;
- do not force an oversized insert into a thin wall;
- keep matching screw sizes for the inserts you actually install.

## Material and print tolerance

Use a material and print settings that keep small holes and spring seats dimensionally stable. Calibrate flow, hole compensation, and elephant-foot compensation before printing final parts. If your printer tends to enlarge holes, plan for M3 hardware in the affected locations.

## Magnets, springs, and steel balls

The build uses 6×2 mm round magnets, compression and extension springs, and steel balls. Keep magnets oriented consistently and test the cap return-to-center behavior before closing the case. Springs should move freely without scraping printed walls.

## Pre-final fit check

Before tightening everything:

1. Confirm the TLV493D board is fixed and aligned with the magnet motion.
2. Check that the cap moves freely in all directions and returns to center.
3. Verify no spring, magnet, screw, or insert can touch the sensor board.
4. Connect USB and confirm serial output before final closing.
