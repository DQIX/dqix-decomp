# Decompiling
## The basics
Decide on a piece of code you want to decompile; either from looking at already decompiled code and wishing to decompile functions it references, or through other means such as debugging. Once you have it, Ghidra can be an excellent base for understanding what the code is trying to achieve, and decomp.me can ensure the code you write matches the assembly.

## Referencing functions and data that have yet to be decompiled
Add a declaration to the file referencing the yet un-decompiled info. For example, if you wanted to reference a function in the main ARM9 file at 02074388, you would add
```C
extern float func_02074388(void*,float,float);
```
so you can reference it. The types may need inferencing, and the function name can be derived from the symbols.txt file under config.

## Creating new files
Once you've decompiled a function and it matches up, it's time to add it to the game. If a file hasn't already been created for this, or it needs tweaking, you need to open the delinks.txt file for the binary you're decompiling part of (ie. ARM9 main, ITCM, or an overlay) and add the file with the start and end address for the file to be inserted into. Example:
```
src/BasicAttackCalculation.cpp:
    complete
    .text start:0x020744c0 end:0x020745ec
```
Add the 'complete' qualifier to indicate the code is finished, or else it won't be included in the build.

Additionally, be sure to rename any symbols in symbols.txt to match the names you gave functions or data in the decompiled file.
