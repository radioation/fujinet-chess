*IMP* the MAP itself uses the upper 4 bits to select the palette
    for a tile in 16-color mode. So for two 4-bit backgrounds using
    diffeernt palets use `-mp` followed by the palette number
   to specify at compile time

```bash
grit chessboard.png  -gB4 -mR4 -mLs -pn16 -ftc -mp 0
grit chesspieces.png -gt -gB4 -Mw2 -Mh2 -pn16 -ftc
grit cursor.png -gt -gB4 -Mw2 -Mh2 -pn16 -ftc
```
