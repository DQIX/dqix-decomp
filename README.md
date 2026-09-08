# Dragon Quest IX: Sentinels of the Starry Skies Decompilation Project

## 📖 About
This project aims to create a **1:1 disassembly and decompilation** of *Dragon Quest IX: Sentinels of the Starry Skies* for the Nintendo DS.  
The primary focus is on the USA and Japanese versions of the game, with the goal of making it fully recompilable.

---

## ⚙️ Setup

### 🛠️ Prerequisites
#### Programs
[Python 3.11 or newer](https://www.python.org/downloads/)

GCC 9+ (available through installers like MINGW on windows, usually included on linux)

[Ninja build system](https://github.com/ninja-build/ninja/releases)

#### Setup
Place a clean ROM of the original game in the 'extract' folder and name it <baserom_dqix_(region).nds>, with (region) being the appropriate region identifier. ex: baserom_dqix_usa.nds

(Note: currently only the usa and jpn roms are supported)

Install the Python dependencies with pip
```shell
python -m pip install -r tools/requirements.txt
```
Run the script to configure Ninja (Do this any time a file is added or removed)
```shell
python tools/configure.py <usa|jpn>
```

If using ghidra, be sure to create a successful build using ninja (see: [Building the Project](#-building-the-project)) at least once before loading the game's config.yaml with ghidra. (For more information see See [Decompiling.md](Decompiling.md))

Lastly, if you want the final ROM to be perfectly byte accurate you need to dump the ARM7 BIOS from your DS and place them in the root folder, under the name arm7_bios.bin

---

### 🚀 Building the Project
Once everything is set up:
1. Open the command line in the root folder of the repository.
2. Run the following command:
   ```bash
   ninja
   ```
This builds the ROM, verifies every module against the original, generates a decomp.me context for each object (requires GCC), creates an objdiff configuration and progress report, and verifies the SHA-1 of the whole built ROM (needs the ARM7 BIOS dump). If you're only interested in building the ROM, you can instead use `ninja min`. This will still verify the modules but not the final SHA-1 and as such does not require GCC or the ARM7 BIOS dump.

---

## 🤝 Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for the full guide.

We recommend joining the DQIX discord server **The Quester's Rest** (https://discord.gg/DQIX) so you can participate in discussions in the **[DQI-haX: SWEs of the Starry Skies](https://discord.com/channels/655390550698098700/1266135635014582332)** thread to claim a function, ask for help, or share a scratch.

### 📤 Submitting Contributions

> [!Important]
> Ensure the decompiled code you submit produces the **same binary** as the original release game. The build script should throw errors should your code not match.

### Decompiling code
See [Decompiling.md](Decompiling.md)

Contributions are welcome, though make sure you've read the pages linked above first. If you're new to decompilation, some possible tasks to get started with include:
1. Find and decompile additional uses for [the game's scripting system](src/Resource/Script.cpp). Some existing uses can be found [here](src/World/LootableContainer.cpp) and [here](src/Graphics/AtmosphericEffect.cpp) as a guideline. (You don't need to worry about matching global variables, it's okay to mark everything as extern for the time being - just focus on matching the .text section). You may find [this script disassembler](https://github.com/DQIX/dqix-script-disasm) helpful.
2. Help to match additional pieces of the Zone3D class. This is very experimental/WIP, so we have [a separate branch](https://github.com/DQIX/dqix-decomp/tree/zone3d-experimental) to work off of for this.
3. Start decompiling one of the game's standalone overlays/systems, such as overlay 6 (the alchemy pot), overlay 8 (battle records) or overlay 14 (bestiary). This will likely interact with some as of yet unknown systems e.g. input, sound or text processing, but partial progress on the rest would still be very interesting!

---

### 🔧 Useful Tools
1. **Ghidra** (with the NTRGhidra plugin):  
   - A powerful reverse engineering tool for DS games and code.  
   - [Download Ghidra](https://github.com/NationalSecurityAgency/ghidra/releases/tag/Ghidra_11.2.1_build) 
   - [Get dsd-ghidra Plugin (use version 0.5.0 for compatibility with dsd 0.10.2)](https://github.com/AetiasHax/dsd-ghidra/releases/tag/v0.5.0)
   
2. **Desmume**:  
   - A DS emulator with excellent debugging features.  
   - [Download nightly builds](https://desmume.org/download/)

3. **No$GBA**:  
   - Another popular DS emulator for debugging, though less user-friendly.  
   - [Download No$GBA](https://problemkaputt.de/gba.htm)

4. **Decomp.me**:
   - A powerful website designed to aid decompilation of games on different platforms. Select the DS platform and input the assembly of the function you want to decompile, and it will show you how closely the code you write matches the output assembly.
   - Additionally, it's great for collaboration, as you can share a "scratch" of the function you're working on with others and they can seamlessly fork it and contribute.
   - [Check it out here](https://decomp.me)

