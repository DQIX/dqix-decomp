#!/usr/bin/env python3

import os
from pathlib import Path
import argparse
import sys

import ninja_syntax
from get_platform import get_platform


DEFAULT_WIBO_PATH = "./wibo"


parser = argparse.ArgumentParser(description="Generates build.ninja")
parser.add_argument('-w', type=str, default=DEFAULT_WIBO_PATH, dest="wine", required=False, help="Path to Wine/Wibo (linux only)")
parser.add_argument("--compiler", type=Path, required=False, help="Path to pre-installed compiler root directory")
parser.add_argument("--no-extract", action="store_true", help="Skip extract step")
parser.add_argument("--dsd", type=Path, required=False, help="Path to pre-installed dsd CLI")
parser.add_argument('version', help='Game version')
args = parser.parse_args()


# Config
GAME = "dqix"
DSD_VERSION = 'v0.6.0'
WIBO_VERSION = '0.6.16'
OBJDIFF_VERSION = 'v2.7.1'
MWCC_VERSION = "2.0/sp1p5"
DECOMP_ME_COMPILER = "mwcc_30_131"
CC_FLAGS = " ".join([
    "-O4,p",                # Optimize maximally for performance
    "-enum int",            # Use int-sized enums
    "-char signed",         # Char type is signed
    "-str noreuse",         # Equivalent strings are different objects
    "-proc arm946e",        # Target processor
    "-gccext,on",           # Enable GCC extensions
    "-fp soft",             # Compute float operations in software
    "-inline noauto",       # Inline only functions marked with 'inline'
    "-lang=c++",            # Set language to C++
    "-Cpp_exceptions off",  # Disable C++ exceptions
    "-RTTI off",            # Disable runtime type information
    "-interworking",        # Enable ARM/Thumb interworking
    "-w off",               # Disable warnings
    "-sym on",              # Debug info, including line numbers
    "-gccinc",              # Interpret #include "..." and #include <...> equally
    "-nolink",              # Do not link
    "-msgstyle gcc",        # Use GCC-like messages (some IDEs will make file names clickable)
])
LD_FLAGS = " ".join([
    "-proc arm946e",        # Target processor
    "-nostdlib",            # No C/C++ standard library
    "-interworking",        # Enable ARM/Thumb interworking
    "-m Entry",             # Set entry function
    "-map closure,unused",  # Generate map file
    "-msgstyle gcc",        # Use GCC-like messages (some IDEs will make file names clickable)
    "-nodead",              # allow dead code
])
DSD_OBJDIFF_ARGS = " ".join([
    "--scratch",                        # Metadata for creating decomp.me scratches
    f"--compiler {DECOMP_ME_COMPILER}", # decomp.me compiler name
    f'--c-flags "{CC_FLAGS} -lang=c++"',# decomp.me compiler flags
    "--custom-make ninja",              # Command for rebuilding files
])


# Paths
current_path     = Path(__name__)
root_path        = current_path.parent
build_ninja_path = root_path / "build.ninja"
arm7_bios_path   = root_path / "arm7_bios.bin"
config_path      = root_path / "config"
build_path       = root_path / "build"
src_path         = root_path / "src"
libs_path        = root_path / "libs"
extract_path     = root_path / "extract"
tools_path       = root_path / "tools"
mwcc_root        = args.compiler or tools_path / "mwccarm"
mwcc_path        = mwcc_root / MWCC_VERSION


# Includes
includes = [
    str(root_path / "include")
]
for root, dirs, _ in os.walk(libs_path):
    for dir in dirs:
        if dir == "include":
            includes.append(Path(root) / dir)
CC_INCLUDES = " ".join(f"-i {include}" for include in includes)


# Platform info
platform = get_platform()
if platform is None:
    exit(1)
EXE = platform.exe
WINE = args.wine if platform.system != "windows" else ""
DSD = str(args.dsd or os.path.join('.', str(root_path / f"dsd{EXE}")))
OBJDIFF = os.path.join('.', str(root_path / f"objdiff-cli{EXE}"))
CC = os.path.join('.', str(mwcc_path / "mwccarm.exe"))
LD = os.path.join('.', str(mwcc_path / "mwldarm.exe"))
PYTHON = sys.executable


class Project:
    def __init__(self, game_version: str):
        self.game_version = game_version
        '''Version of the game'''
        self.game_config = config_path / game_version
        '''Root directory for dsd configs'''

        if not self.game_config.is_dir():
            print(f"Version '{game_version}' not recognized")
            exit(1)

        self.game_build = build_path / game_version
        '''Path to build directory'''
        self.game_extract = extract_path / game_version
        '''Path to extract directory'''

        self.delinks_files = get_config_files(self.game_config, "delinks.txt")
        '''Paths to every delinks.txt file'''
        self.relocs_files = get_config_files(self.game_config, "relocs.txt")
        '''Paths to every relocs.txt file'''
        self.symbols_files = get_config_files(self.game_config, "symbols.txt")
        '''Paths to every symbols.txt file'''

    def dsd_configs(self) -> list[str]:
        return self.delinks_files + self.relocs_files + self.symbols_files

    def arm9_config_yaml(self) -> Path:
        return self.game_config / "arm9" / "config.yaml"

    def baserom(self) -> Path:
        return extract_path / f'baserom_{GAME}_{self.game_version}.nds'

    def build_rom(self) -> str:
        return f"{GAME}_{self.game_version}.nds"

    def baserom_config(self) -> Path:
        return self.game_extract / 'config.yaml'

    def build_rom_config(self) -> Path:
        return self.game_build / "build" / "rom_config.yaml"

    def source_object_files(self) -> list[str]:
        return [
            str(self.game_build / source_file.with_suffix(".o"))
            for source_file in get_c_cpp_files([src_path, libs_path])
        ]

    def arm9_lcf(self) -> Path:
        return self.game_build / "linker_script.lcf"

    def arm9_objects_txt(self) -> Path:
        return self.game_build / "objects.txt"

    def arm9_delink_yaml(self) -> Path:
        return self.game_build / "delinks" / "delink.yaml"

    def arm9_o(self) -> Path:
        return self.game_build / "arm9.o"

    def arm9_delinks(self) -> Path:
        return self.game_build / "delinks"

    def objdiff_report(self) -> Path:
        return self.game_build / "report.json"


def main():
    project = Project(args.version)

    with build_ninja_path.open("w") as file:
        n = ninja_syntax.Writer(file)

        n.rule(
            name="download_tool",
            command=f'{PYTHON} tools/download_tool.py $tool $tag --path $path'
        )
        n.newline()

        if arm7_bios_path.is_file():
            n.variable("arm7_bios_flag", f"--arm7-bios {arm7_bios_path.relative_to(root_path)}")
        else:
            n.variable("arm7_bios_flag", "")
        n.newline()

        n.rule(
            name="extract",
            command=f"{DSD} rom extract --rom $in --output-path $output_path $arm7_bios_flag"
        )
        n.newline()

        n.rule(
            name="delink",
            command=f"{DSD} delink --config-path $config_path"
        )
        n.newline()

        # -MMD excludes all includes instead of just system includes for some reason, so use -MD instead.
        mwcc_cmd = f'{WINE} "{CC}" {CC_FLAGS} {CC_INCLUDES} $cc_flags -d $game_version -MD -c $in -o $basedir'
        mwcc_implicit = [CC]
        if platform.system != "windows":
            transform_dep = "tools/transform_dep.py"
            mwcc_cmd += f" && $python {transform_dep} $basefile.d $basefile.d"
            mwcc_implicit.append(transform_dep)
        n.rule(
            name="mwcc",
            command=mwcc_cmd,
            depfile="$basefile.d",
        )
        n.newline()

        n.rule(
            name="lcf",
            command=f"{DSD} lcf -c $config_path --lcf-file $lcf_file --objects-file $objects_file"
        )
        n.newline()

        n.rule(
            name="mwld",
            command=f'{WINE} "{LD}" {LD_FLAGS} @$objects_file $lcf_file -o $out'
        )
        n.newline()

        n.rule(
            name="rom_config",
            command=f"{DSD} rom config --elf $in --config $config_path"
        )
        n.newline()

        n.rule(
            name="rom_build",
            command=f"{DSD} rom build --config $in --rom $out $arm7_bios_flag"
        )
        n.newline()

        n.rule(
            name="objdiff",
            command=f"{DSD} objdiff --config-path $config_path {DSD_OBJDIFF_ARGS}"
        )
        n.newline()

        n.rule(
            name="objdiff_report",
            command=f"{OBJDIFF} report generate -o $out"
        )
        n.newline()

        n.rule(
            name="m2ctx",
            command=f"{PYTHON} tools/m2ctx.py -f $out $in"
        )
        n.newline()

        n.rule(
            name="check_modules",
            command=f"{DSD} check modules --config-path $config_path --fail"
        )
        n.newline()

        n.rule(
            name="check_symbols",
            command=f"{DSD} check symbols --config-path $config_path --elf-path $elf_path --fail"
        )
        n.newline()

        n.rule(
            name="sha1",
            command=f"{PYTHON} tools/sha1.py $in -c $sha1_file"
        )
        n.newline()

        add_download_tool_builds(n)
        add_extract_build(n, project)
        add_delink_and_lcf_builds(n, project)
        add_mwcc_builds(n, project, mwcc_implicit)
        add_mwld_and_rom_builds(n, project)
        add_check_builds(n, project)
        add_objdiff_builds(n, project)


def add_download_tool_builds(n: ninja_syntax.Writer):
    if args.dsd is None:
        n.build(
            rule="download_tool",
            outputs=DSD,
            variables={
                "tool": "dsd",
                "tag": DSD_VERSION,
                "path": DSD,
            },
        )
        n.newline()

    n.build(
        rule="download_tool",
        outputs=OBJDIFF,
        variables={
            "tool": "objdiff",
            "tag": OBJDIFF_VERSION,
            "path": OBJDIFF,
        }
    )
    n.newline()

    if args.compiler is None:
        n.build(
            rule="download_tool",
            outputs=[CC, LD],
            variables={
                "tool": "mwccarm",
                "tag": "latest",
                "path": tools_path,
            },
        )
        n.newline()

    if platform.system != "windows" and WINE == DEFAULT_WIBO_PATH:
        n.build(
            rule="download_tool",
            outputs=WINE,
            variables={
                "tool": "wibo",
                "tag": WIBO_VERSION,
                "path": WINE,
            },
        )
        n.newline()


def add_extract_build(n: ninja_syntax.Writer, project: Project):
    if not args.no_extract:
        n.build(
            inputs=str(project.baserom()),
            implicit=DSD,
            rule="extract",
            outputs=str(project.baserom_config()),
            variables={
                "output_path": str(project.game_extract)
            }
        )
        n.newline()


def add_mwld_and_rom_builds(n: ninja_syntax.Writer, project: Project):
    lcf_file = str(project.arm9_lcf())
    objects_file = str(project.arm9_objects_txt())
    delink_file = str(project.arm9_delink_yaml())
    elf_file = str(project.arm9_o())
    n.build(
        inputs=project.source_object_files() + [lcf_file, objects_file, delink_file],
        implicit=LD,
        rule="mwld",
        outputs=elf_file,
        variables={
            "target_dir": project.game_build,
            "objects_file": objects_file,
            "lcf_file": lcf_file,
        }
    )
    n.newline()

    n.build(
        inputs=elf_file,
        rule="phony",
        outputs="arm9",
    )
    n.newline()

    rom_config_file = str(project.build_rom_config())
    n.build(
        inputs=elf_file,
        implicit=DSD,
        rule="rom_config",
        outputs=rom_config_file,
        variables={
            "config_path": project.arm9_config_yaml(),
        }
    )
    n.newline()

    rom_file = project.build_rom()
    n.build(
        inputs=rom_config_file,
        implicit=DSD,
        rule="rom_build",
        outputs=rom_file,
    )
    n.newline()

    n.build(
        inputs=rom_file,
        rule="phony",
        outputs="rom",
    )
    n.newline()

    n.build(
        inputs=rom_file,
        rule="sha1",
        variables={
            "sha1_file": str(Path(rom_file).with_suffix(".sha1"))
        },
        outputs="sha1",
    )
    n.newline()


def add_mwcc_builds(n: ninja_syntax.Writer, project: Project, mwcc_implicit: list[Path]):
    for source_file in get_c_cpp_files([src_path, libs_path]):
        src_obj_path = project.game_build / source_file
        cc_flags = []
        if is_cpp(source_file): cc_flags.append("-lang=c++")
        elif is_c(source_file): cc_flags.append("-lang=c")
        n.build(
            inputs=str(source_file),
            implicit=mwcc_implicit,
            rule="mwcc",
            outputs=str(src_obj_path.with_suffix(".o")),
            variables={
                "game_version": project.game_version,
                "cc_flags": " ".join(cc_flags),
                "basedir": os.path.dirname(src_obj_path),
                "basefile": str(src_obj_path.with_suffix("")),
            },
        )
        n.newline()

        extension = source_file.suffix
        ctx_file = str(project.game_build / source_file.with_suffix(f".ctx{extension}"))
        n.build(
            inputs=str(source_file),
            rule="m2ctx",
            outputs=ctx_file,
        )
        n.newline()


def get_c_cpp_files(dirs: list[Path]):
    for dir in dirs:
        for root, _, files in os.walk(dir):
            root = Path(root)
            for file in files:
                if is_cpp(file) or is_c(file):
                    yield root / file


def is_cpp(name: str):
    return Path(name).suffix in [".cpp"]


def is_c(name: str):
    return Path(name).suffix in [".c"]


def add_delink_and_lcf_builds(n: ninja_syntax.Writer, project: Project):
    n.comment("Delink ELF binaries when any delinks.txt file is modified")
    rom_config = str(project.baserom_config())
    delinks_path = project.arm9_delinks()
    n.build(
        inputs=project.dsd_configs() + [rom_config],
        implicit=DSD,
        rule="delink",
        outputs=str(delinks_path / "delink.yaml"),
        variables={
            "config_path": project.arm9_config_yaml(),
        }
    )
    n.newline()

    n.build(
        inputs=str(delinks_path / "delink.yaml"),
        rule="phony",
        outputs="delink"
    )
    n.newline()

    lcf_file = project.arm9_lcf()
    objects_file = project.arm9_objects_txt()
    n.build(
        inputs=project.delinks_files + [str(rom_config)],
        implicit=DSD,
        rule="lcf",
        outputs=[str(lcf_file), str(objects_file)],
        variables={
            "config_path": project.arm9_config_yaml(),
            "lcf_file": lcf_file,
            "objects_file": objects_file,
        }
    )
    n.newline()


def add_check_builds(n: ninja_syntax.Writer, project: Project):
    n.build(
        inputs=str(project.arm9_o()),
        rule="check_modules",
        outputs="check_modules",
        variables={
            "config_path": project.arm9_config_yaml(),
        },
    )
    n.newline()

    n.build(
        inputs=str(project.arm9_o()),
        rule="check_symbols",
        outputs="check_symbols",
        variables={
            "config_path": project.arm9_config_yaml(),
            "elf_path": project.arm9_o(),
        },
    )
    n.newline()

    n.build(
        inputs=["check_modules", "check_symbols"],
        rule="phony",
        outputs="check",
    )
    n.newline()


def add_objdiff_builds(n: ninja_syntax.Writer, project: Project):
    n.build(
        inputs=project.dsd_configs(),
        implicit=DSD,
        rule="objdiff",
        outputs="objdiff.json",
        variables={
            "config_path": project.arm9_config_yaml(),
        }
    )
    n.newline()

    n.build(
        inputs="objdiff.json",
        rule="phony",
        outputs="objdiff",
    )
    n.newline()

    n.build(
        inputs=["objdiff.json"],
        implicit=[OBJDIFF] + project.source_object_files(),
        rule="objdiff_report",
        outputs=str(project.objdiff_report()),
    )
    n.newline()

    n.build(
        inputs=str(project.objdiff_report()),
        rule="phony",
        outputs="report",
    )
    n.newline()


def get_config_files(game_config: Path, name: str) -> list[str]:
    return [
        f"{root}/{file}"
        for root, _, files in os.walk(game_config)
        for file in files
        if file == name
    ]


if __name__ == "__main__": main()
