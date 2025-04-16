# Dragon Quest IX: Sentinels of the Starry Skies Decompilation Project

## 📖 About
This project aims to create a **1:1 disassembly and decompilation** of *Dragon Quest IX: Sentinels of the Starry Skies* for the Nintendo DS.  
The primary focus is on the USA version of the game, with the goal of making it fully recompilable.

---

## ⚙️ Setup

### 🛠️ Prerequisites
#### Programs
[Python 3.11 or newer](https://www.python.org/downloads/)

GCC 9+ (available through installers like MINGW on windows, usually included on linux)

[Ninja build system](https://github.com/ninja-build/ninja/releases)

#### Setup
Place a clean ROM of the original game in the 'extract' folder and name it <baserom_dqix_(region).nds>, with (region) being the appropriate region identifier. ex: baserom_dqix_usa.nds

(Note: currently only the usa rom is supported)
Install the Python dependencies with pip
```shell
python -m pip install -r tools/requirements.txt
```
Run the script to configure Ninja (Do this any time a file is added or removed)
```shell
python tools/configure.py usa
```
Lastly, if you want the final ROM to be perfectly byte accurate you need to dump the ARM7 BIOS from your DS and place them in the root folder, under the name arm7_bios.bin

---

### 🚀 Building the Project
Once everything is set up:
1. Open the command line in the root folder of the repository.
2. Run the following command:
   ```bash
   ninja
   ```
This will initiate the compilation process.

---

## 🤝 Contributing

### 📤 Submitting Contributions

> [!Important]
> Ensure the decompiled code you submit produces the **same binary** as the original release game. The build script should throw errors should your code not match.

### Decompiling code
See [Decompiling.md](Decompiling.md)

---

### 🧑‍💻 Current Goals
Disassemble and decompile accessible functions to begin mapping out the games code.

---

### 🔧 Useful Tools
1. **Ghidra** (with the NTRGhidra plugin):  
   - A powerful reverse engineering tool for DS games and code.  
   - [Download Ghidra](https://github.com/NationalSecurityAgency/ghidra/releases)  
   - [Get NTRGhidra Plugin](https://github.com/pedro-javierf/NTRGhidra/releases)
   
2. **Desmume**:  
   - A DS emulator with excellent debugging features.  
   - [Download nightly builds](https://desmume.org/download/)

3. **No$GBA**:  
   - Another popular DS emulator for debugging, though less user-friendly.  
   - [Download No$GBA](https://problemkaputt.de/gba.htm)

---

### ✨ Notes
This project is a work in progress. Community contributions and feedback are welcome to help improve functionality!
