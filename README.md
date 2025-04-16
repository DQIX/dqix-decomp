# Dragon Quest IX: Sentinels of the Starry Skies Decompilation Project

## 📖 About
This project aims to create a **1:1 disassembly and decompilation** of *Dragon Quest IX: Sentinels of the Starry Skies* for the Nintendo DS.  
The primary focus is on the USA version of the game, with the goal of making it fully recompilable.

---

## ⚙️ Setup

### 🛠️ Prerequisites
#### Installing `make`
- **Windows**:  
  - Install `make` by using [MSYS2](https://www.msys2.org/).
  - Alternatively, if you develop Homebrew applications, it comes bundled with DevkitPro installation.
 
- **macOS**:  
  - Install Xcode Command Line Tools, which include `make`:
    ```bash
    xcode-select --install
    ```
  - Alternatively, if you use Homebrew:
    ```bash
    brew install make
    ```

- **Linux**:
  - `make` is usually pre-installed. If not, install it using your package manager:
    - Debian/Ubuntu-based systems:
      ```bash
      sudo apt update && sudo apt install make
      ```
    - Red Hat-based systems (Fedora, CentOS):
      ```bash
      sudo dnf install make
      ```
    - Arch-based systems:
      ```bash
      sudo pacman -S make
      ```

#### Compiler
- Download the compiler [**mwccarm.zip**](http://decomp.aetias.com/files/mwccarm.zip).
- Extract its contents to a folder named `tools` in the root directory of the project.

#### ROM Setup
*(TODO: The `Makefile` currently only produces the executable, not the ROM)*

---

### 🚀 Building the Project
Once everything is set up:
1. Open the command line in the root folder of the repository.
2. Run the following command:
   ```bash
   make
   ```
This will initiate the compilation process.

---

## 🤝 Contributing

### 📤 Submitting Contributions

> [!Important]
> Ensure the decompiled code you submit produces the **same binary** as the original release game.  
*(TODO: Implement a hashing mechanism to verify this.)*

#### Known Issue
- The binary currently doesn't match the original game due to unresolved issues with `.bss` sections. *(Help fix this!)*

---

### 🧑‍💻 Current Goals
The immediate objective is to **fix the `.bss` sections** in the disassembly:
- At the end of `main.s`, there are numerous labels meant for a `.bss` section.
- These labels need to be matched with their correct memory positions, either in `main.s` or the relevant overlay files.

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
This project is a work in progress. Community contributions and feedback are welcome to help improve accuracy and functionality!
