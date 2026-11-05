#!/usr/bin/env python3

import argparse
from pathlib import Path
import re
import shutil
import struct
import sys


parser = argparse.ArgumentParser(description="Ports config/usa to config/eur")
parser.add_argument("--usa", type=Path, default=Path("config/usa"), help="USA config directory")
parser.add_argument("--eur", type=Path, default=Path("config/eur"), help="EUR config directory to create")
parser.add_argument("--extract", type=Path, default=Path("extract/eur"), help="Extracted EUR ROM directory")
parser.add_argument("--force", action="store_true", help="Overwrite the EUR config directory if it exists")
parser.add_argument("--sync", action="store_true",
                    help="Bring the USA names and matched files config/eur lacks into it, keeping its own work")
parser.add_argument("--dry-run", action="store_true", help="With --sync, report without writing")
args = parser.parse_args()


SECURE_AREA_START = 0x02000000
SECURE_AREA_END   = 0x02000800

MAIN_SHIFTS = [
    (0x02000800, 0x0200f3c8, 0x0),
    (0x0200f3c8, 0x0200fb24, 0x8),
    (0x0200fb24, 0x020eebe0, 0x10),
    (0x020eebe0, 0x021536e0, 0x0),
]

BIOS_STUBS = {
    "SoftReset": 0x00,
    "WaitByLoop": 0x03,
    "IntrWait": 0x04,
    "VBlankIntrWait": 0x05,
    "Halt": 0x06,
    "Div": 0x09,
    "Mod": 0x09,
    "CpuSet": 0x0b,
    "CpuFastSet": 0x0c,
    "Sqrt": 0x0d,
    "GetCRC16": 0x0e,
    "IsDebugger": 0x0f,
    "BitUnPack": 0x10,
    "LZ77UnCompReadNormalWrite8bit": 0x11,
    "LZ77UnCompReadByCallbackWrite16bit": 0x12,
    "HuffUnCompReadByCallback": 0x13,
    "RLUnCompReadNormalWrite8bit": 0x14,
    "RLUnCompReadByCallbackWrite16bit": 0x15,
}

THUMB_BX_LR = 0x4770
THUMB_MOV_R0_R1 = 0x1c08

STUBS_WITH_MOV_R2_0 = {"IntrWait", "VBlankIntrWait"}

from region_port import (DELINK_END, DELINK_START, RELOC, SYMBOL_ADDR, SYMBOL_SIZE, SameAddress,
                         block_ranges, branch_target, delink_blocks, hex_address, is_local, port_delink,
                         read_raw, reloc_sources, symbol_lines, sync_delinks, thumb_branch_target, with_local)


class MainAddressMap:
    def __init__(self, usa_symbols: Path, eur_arm9_bin: Path):
        self.arm9 = eur_arm9_bin.read_bytes()
        self.stubs = self.find_bios_stubs(usa_symbols)

    def find_bios_stubs(self, usa_symbols: Path) -> dict[int, int]:
        eur_stubs = {}
        for address in range(SECURE_AREA_START, SECURE_AREA_END - 4, 2):
            swi = self.halfword(address)
            if swi >> 8 != 0xdf:
                continue
            if self.halfword(address + 2) == THUMB_BX_LR:
                eur_stubs[(swi & 0xff, False)] = address
            elif self.halfword(address + 2) == THUMB_MOV_R0_R1 and self.halfword(address + 4) == THUMB_BX_LR:
                eur_stubs[(swi & 0xff, True)] = address

        stubs = {}
        for name, address, _ in read_symbols(usa_symbols):
            if not SECURE_AREA_START <= address < SECURE_AREA_END:
                continue
            if name not in BIOS_STUBS:
                continue
            key = (BIOS_STUBS[name], name == "Mod")
            if key not in eur_stubs:
                sys.exit(f"BIOS call stub '{name}' not found in the EUR secure area")
            stubs[address] = eur_stubs[key] - 2 * (name in STUBS_WITH_MOV_R2_0)
        self.stub_addrs = sorted(stubs)
        return stubs

    def halfword(self, address: int) -> int:
        return struct.unpack_from("<H", self.arm9, address - SECURE_AREA_START)[0]

    def map(self, address: int) -> int:
        if address in self.stubs:
            return self.stubs[address]
        if SECURE_AREA_START < address < SECURE_AREA_END and self.stub_addrs:
            nearest = min(self.stub_addrs, key=lambda usa: abs(address - usa))
            if abs(address - nearest) <= 0x10:
                return self.stubs[nearest] + (address - nearest)
        if address == SECURE_AREA_START:
            return address
        for start, end, shift in MAIN_SHIFTS:
            if start <= address < end:
                return address + shift
        raise ValueError(f"No EUR equivalent known for main address {address:#010x}")

    def map_end(self, end: int) -> int:
        return self.map(end - 1) + 1


def read_symbols(path: Path):
    for line in path.read_text().splitlines():
        match = SYMBOL_ADDR.search(line)
        if match:
            size = SYMBOL_SIZE.search(line)
            yield line.split(" ", 1)[0], int(match[1], 16), int(size[1], 16) if size else None


def port_lines(path: Path, port_line):
    with path.open("r", encoding="utf-8", newline="") as file:
        lines = file.read().splitlines(keepends=True)
    with path.open("w", encoding="utf-8", newline="") as file:
        file.writelines(port_line(line) for line in lines)


def port_symbol(line: str, main: MainAddressMap) -> str:
    match = SYMBOL_ADDR.search(line)
    if not match:
        return line
    address = int(match[1], 16)
    new_address = main.map(address)
    size = SYMBOL_SIZE.search(line)
    if size and int(size[1], 16) > 0 and address >= SECURE_AREA_END:
        new_size = main.map_end(address + int(size[1], 16)) - new_address
        line = SYMBOL_SIZE.sub(f"size={new_size:#x}", line)
    return SYMBOL_ADDR.sub(f"addr:{hex_address(new_address)}", line)


def word_is_moved_pointer(main: MainAddressMap, usa_word: int, eur_word: int) -> bool:
    if not SECURE_AREA_START <= usa_word < 0x02400000:
        return False
    if not SECURE_AREA_START <= eur_word < 0x02400000:
        return False
    try:
        return main.map(usa_word) == eur_word
    except ValueError:
        return False


def range_reproducible(usa_bin: bytes, eur_bin: bytes, usa_start: int, usa_end: int,
                       eur_start: int, eur_end: int, reloc_froms: list[int],
                       main: MainAddressMap, base: int) -> bool:
    if eur_end - eur_start != usa_end - usa_start:
        return False
    if usa_start < base or eur_start < base:
        return False
    usa_off = usa_start - base
    eur_off = eur_start - base
    size = usa_end - usa_start
    if usa_off >= len(usa_bin) and eur_off >= len(eur_bin):
        return True
    if usa_off < 0 or eur_off < 0 or usa_off + size > len(usa_bin) or eur_off + size > len(eur_bin):
        return False
    usa = usa_bin[usa_off:usa_off + size]
    eur = eur_bin[eur_off:eur_off + size]
    mask = bytearray(size)
    for src in reloc_froms:
        if usa_start <= src < usa_end:
            off = src - usa_start
            for i in range(off, min(off + 4, size)):
                mask[i] = 1
    for off in range(0, size - 3):
        if (usa_start + off) & 3:
            continue
        usa_word = struct.unpack_from("<I", usa, off)[0]
        eur_word = struct.unpack_from("<I", eur, off)[0]
        if usa_word != eur_word and word_is_moved_pointer(main, usa_word, eur_word):
            for i in range(off, off + 4):
                mask[i] = 1
    return all(mask[i] or usa[i] == eur[i] for i in range(size))


def port_main_delinks(path: Path, main: MainAddressMap, usa_bin: bytes, eur_bin: bytes,
                      reloc_froms: list[int]) -> int:
    header, blocks = delink_blocks(read_raw(path))
    kept = []
    dropped = 0
    for block in blocks:
        ranges = block_ranges(block)
        ok = True
        for usa_start, usa_end in ranges:
            eur_start = main.map(usa_start)
            eur_end = main.map_end(usa_end)
            if eur_start & 3 or eur_end & 3:
                ok = False
                break
            if not range_reproducible(usa_bin, eur_bin, usa_start, usa_end, eur_start, eur_end, reloc_froms, main, SECURE_AREA_START):
                ok = False
                break
        if ok:
            kept.extend(port_delink(line, main) for line in block)
        else:
            dropped += 1
            name = block[0].strip()
            print(f"  ROM bytes: {name}")
    path.write_text("".join(port_delink(line, main) for line in header) + "".join(kept), encoding="utf-8", newline="")
    return dropped


def port_same_address_delinks(path: Path, usa_bin: bytes, eur_bin: bytes, base: int,
                             reloc_froms: list[int], main: MainAddressMap) -> int:
    header, blocks = delink_blocks(read_raw(path))
    kept = []
    dropped = 0
    for block in blocks:
        ok = True
        for start, end in block_ranges(block):
            if start & 3 or end & 3:
                ok = False
                break
            if not range_reproducible(usa_bin, eur_bin, start, end, start, end, reloc_froms, main, base):
                ok = False
                break
        if ok:
            kept.extend(block)
        else:
            dropped += 1
            print(f"  ROM bytes: {block[0].strip()}")
    path.write_text("".join(header) + "".join(kept), encoding="utf-8", newline="")
    return dropped


def port_reloc(line: str, main: MainAddressMap, from_main: bool) -> str:
    match = RELOC.search(line)
    if not match:
        return line
    start, end = match.span()
    source, kind, target, addend, module = match.groups()
    source = int(source, 16)
    target = int(target, 16)
    if from_main:
        source = main.map(source)
    if module == "main":
        target = main.map(target)
    ported = f"from:{hex_address(source)} kind:{kind} to:{hex_address(target)}"
    if addend is not None:
        ported += f" add:{addend}"
    ported += f" module:{module}"
    return line[:start] + ported + line[end:]


def fxhash64(data: bytes) -> int:
    mask = (1 << 64) - 1
    def add_word(hash: int, word: int) -> int:
        return ((((hash << 5) | (hash >> 59)) & mask) ^ word) * 0x517cc1b727220a95 & mask

    hash = add_word(0, len(data))
    offset = 0
    for size, format in [(8, "<Q"), (4, "<I"), (2, "<H"), (1, "<B")]:
        while len(data) - offset >= size:
            hash = add_word(hash, struct.unpack_from(format, data, offset)[0])
            offset += size
            if size < 8:
                break
    return hash


def extracted_module(object_path: str) -> Path:
    name = Path(object_path.strip("'\"")).name
    if name.startswith("arm9_ov"):
        return args.extract / "arm9_overlays" / name.removeprefix("arm9_")
    return args.extract / "arm9" / name


def port_config_yaml(path: Path):
    object_path = None
    def port_line(line: str) -> str:
        nonlocal object_path
        key, _, value = line.strip().partition(": ")
        if key in ["rom_config", "build_path", "delinks_path", "object"]:
            line = re.sub(r"/usa\b", "/eur", line)
        if key == "object":
            object_path = value
        elif key == "hash":
            hash = f"{fxhash64(extracted_module(object_path).read_bytes()):016x}"
            if hash[0].isdigit():
                hash = f"'{hash}'"
            line = line[:line.index("hash: ") + len("hash: ")] + hash + line[len(line.rstrip("\r\n")):]
        return line
    port_lines(path, port_line)


def module_base(delinks: Path) -> int:
    return min(int(start, 16) for start in DELINK_START.findall(delinks.read_text()))


def clamp_secure_data(symbols: Path, delinks: Path) -> None:
    ranges = []
    for line in delinks.read_text(encoding="utf-8").splitlines():
        start = DELINK_START.search(line)
        end = DELINK_END.search(line)
        if not start or not end:
            continue
        start_addr = int(start[1], 16)
        end_addr = int(end[1], 16)
        if start_addr == SECURE_AREA_START and end_addr > SECURE_AREA_END:
            continue
        if start_addr < SECURE_AREA_END:
            ranges.append((start_addr, end_addr))
    ranges.sort()

    def section_end(address: int) -> int:
        for start_addr, end_addr in ranges:
            if start_addr <= address < end_addr:
                return end_addr
        for start_addr, _ in ranges:
            if start_addr > address:
                return start_addr
        return SECURE_AREA_END

    lines = read_raw(symbols).splitlines(keepends=True)
    parsed = []
    for index, line in enumerate(lines):
        match = SYMBOL_ADDR.search(line)
        if not match:
            continue
        address = int(match[1], 16)
        if not SECURE_AREA_START <= address < SECURE_AREA_END:
            continue
        size = re.search(r"byte\[(0x[0-9a-fA-F]+|\d+)\]", line)
        if not size:
            parsed.append((address, None, index))
            continue
        parsed.append((address, int(size[1], 0), index))
    parsed.sort()
    for position, (address, size, index) in enumerate(parsed):
        if size is None:
            continue
        nxt = parsed[position + 1][0] if position + 1 < len(parsed) else SECURE_AREA_END
        room = min(nxt, section_end(address)) - address
        if room < size:
            lines[index] = re.sub(r"byte\[0x[0-9a-fA-F]+|byte\[\d+", f"byte[{room}", lines[index], count=1)
    symbols.write_text("".join(lines), encoding="utf-8", newline="")


def module_binary(module_dir: Path) -> Path:
    if module_dir.parent.name == "overlays":
        return args.extract / "arm9_overlays" / f"{module_dir.name}.bin"
    if module_dir.name in ["itcm", "dtcm"]:
        return args.extract / "arm9" / f"{module_dir.name}.bin"
    return args.extract / "arm9" / "arm9.bin"


def verify_relocs(relocs: Path) -> tuple[int, list[str]]:
    module_dir = relocs.parent
    base = module_base(module_dir / "delinks.txt")
    data = module_binary(module_dir).read_bytes()
    count = 0
    errors = []
    for line in relocs.read_text().splitlines():
        match = RELOC.search(line)
        if not match:
            continue
        count += 1
        source, target = int(match[1], 16), int(match[3], 16)
        addend = int(match[4], 16) if match[4] else 0
        value = struct.unpack_from("<I", data, source - base)[0]
        if match[2] == "load":
            ok = value in [target, target + addend]
        elif match[2].startswith("thumb_"):
            ok = thumb_branch_target(value, source) == target
        else:
            ok = branch_target(value, source) == target
        if not ok:
            errors.append(f"{relocs}: {line.strip()} (found {value:08x})")
    return count, errors


def sync_symbols(usa_path: Path, eur_path: Path, mapper) -> tuple[int, list[str]]:
    usa_lines, usa_at = symbol_lines(usa_path)
    eur_lines, eur_at = symbol_lines(eur_path)
    renamed = 0
    unresolved = []
    for address, usa_indices in usa_at.items():
        try:
            eur_address = mapper.map(address)
        except ValueError:
            continue
        usa_names = [usa_lines[i].split(" ", 1)[0] for i in usa_indices]
        eur_indices = eur_at.get(eur_address, [])
        eur_names = [eur_lines[i].split(" ", 1)[0] for i in eur_indices]
        if set(usa_names) <= set(eur_names):
            for i, name in zip(usa_indices, usa_names):
                index = eur_indices[eur_names.index(name)]
                flagged = with_local(eur_lines[index], is_local(usa_lines[i]))
                if flagged != eur_lines[index]:
                    eur_lines[index] = flagged
                    renamed += 1
            continue
        if eur_names and set(eur_names) < set(usa_names):
            newline = "\r\n" if eur_lines[eur_indices[-1]].endswith("\r\n") else "\n"
            extra = [port_symbol(usa_lines[i], mapper).rstrip("\r\n") + newline
                     for i, name in zip(usa_indices, usa_names) if name not in eur_names]
            eur_lines[eur_indices[-1]] += "".join(extra)
            renamed += len(extra)
        elif len(usa_names) == 1 and len(eur_names) == 1:
            index = eur_indices[0]
            eur_lines[index] = with_local(usa_names[0] + eur_lines[index][len(eur_names[0]):],
                                          is_local(usa_lines[usa_indices[0]]))
            renamed += 1
        else:
            unresolved.append(f"{hex_address(eur_address)} USA {usa_names} EUR {eur_names}")
    if renamed and not args.dry_run:
        eur_path.write_text("".join(eur_lines), encoding="utf-8", newline="")
    return renamed, unresolved


def sync():
    usa_arm9 = args.usa / "arm9"
    eur_arm9 = args.eur / "arm9"
    eur_arm9_bin = args.extract / "arm9" / "arm9.bin"
    if not eur_arm9.is_dir() or not eur_arm9_bin.is_file():
        sys.exit(f"--sync needs {eur_arm9} and an extracted EUR ROM at {args.extract}")
    main_map = MainAddressMap(usa_arm9 / "symbols.txt", eur_arm9_bin)
    usa_extract = args.extract.parent / "usa"
    total_renamed = total_added = 0
    for usa_delinks in sorted(usa_arm9.rglob("delinks.txt")):
        usa_dir = usa_delinks.parent
        eur_dir = args.eur / usa_dir.relative_to(args.usa)
        if not (eur_dir / "delinks.txt").is_file():
            print(f"{eur_dir}: missing")
            continue
        is_main = usa_dir == usa_arm9
        mapper = main_map if is_main else SameAddress()
        eur_bin = module_binary(eur_dir).read_bytes()
        usa_bin = (usa_extract / module_binary(eur_dir).relative_to(args.extract)).read_bytes()
        base = SECURE_AREA_START if is_main else module_base(eur_dir / "delinks.txt")
        sources = reloc_sources(usa_dir / "relocs.txt")

        def reproducible(usa_start, usa_end, eur_start, eur_end):
            return range_reproducible(usa_bin, eur_bin, usa_start, usa_end, eur_start, eur_end,
                                      sources, main_map, base)

        renamed, unresolved = sync_symbols(usa_dir / "symbols.txt", eur_dir / "symbols.txt", mapper)
        added, notes = sync_delinks(usa_delinks, eur_dir / "delinks.txt", mapper, reproducible, "EUR", args.dry_run)
        total_renamed += renamed
        total_added += added
        if renamed or added or notes or unresolved:
            print(f"{eur_dir}: {renamed} names, {added} files")
        for line in unresolved + notes:
            print(f"  {line}")
    print(f"{'Would bring' if args.dry_run else 'Brought'} {total_renamed} names and {total_added} files into {args.eur}")


def main():
    if args.sync:
        return sync()
    usa_arm9 = args.usa / "arm9"
    eur_arm9 = args.eur / "arm9"
    eur_arm9_bin = args.extract / "arm9" / "arm9.bin"

    if not eur_arm9_bin.is_file():
        sys.exit(f"{eur_arm9_bin} not found, extract the EUR ROM first")
    if args.eur.exists():
        if not args.force:
            sys.exit(f"{args.eur} already exists, use --force to overwrite it")
        shutil.rmtree(args.eur)

    main_map = MainAddressMap(usa_arm9 / "symbols.txt", eur_arm9_bin)
    shutil.copytree(args.usa, args.eur)
    usa_bin = (args.extract.parent / "usa" / "arm9" / "arm9.bin").read_bytes()
    reloc_froms = []
    for line in (usa_arm9 / "relocs.txt").read_text(encoding="utf-8", errors="replace").splitlines():
        match = RELOC.search(line)
        if match:
            reloc_froms.append(int(match[1], 16))

    for file in eur_arm9.rglob("*.txt"):
        from_main = file.parent == eur_arm9
        if file.name == "relocs.txt":
            port_lines(file, lambda line: port_reloc(line, main_map, from_main))
        elif from_main and file.name == "symbols.txt":
            port_lines(file, lambda line: port_symbol(line, main_map))
        elif from_main and file.name == "delinks.txt":
            dropped = port_main_delinks(file, main_map, usa_bin, main_map.arm9, reloc_froms)
            print(f"Left {dropped} main ranges as EUR ROM bytes")
        elif file.name == "delinks.txt":
            usa_dir = args.usa / file.parent.relative_to(args.eur)
            eur_bin = module_binary(file.parent)
            usa_module = args.extract.parent / "usa" / eur_bin.relative_to(args.extract)
            module_relocs = []
            relocs_path = usa_dir / "relocs.txt"
            if relocs_path.is_file():
                for line in relocs_path.read_text(encoding="utf-8", errors="replace").splitlines():
                    match = RELOC.search(line)
                    if match:
                        module_relocs.append(int(match[1], 16))
            dropped = port_same_address_delinks(
                file, usa_module.read_bytes(), eur_bin.read_bytes(),
                module_base(file), module_relocs, main_map)
            if dropped:
                print(f"Left {dropped} ranges in {file.parent.name} as EUR ROM bytes")
    port_config_yaml(eur_arm9 / "config.yaml")
    clamp_secure_data(eur_arm9 / "symbols.txt", eur_arm9 / "delinks.txt")
    print(f"Ported {args.usa} to {args.eur}")

    total = 0
    errors = []
    for relocs in sorted(eur_arm9.rglob("relocs.txt")):
        count, module_errors = verify_relocs(relocs)
        total += count
        errors += module_errors
    print(f"Verified {total - len(errors)}/{total} relocations against {args.extract}")
    if errors:
        print("\n".join(errors[:50]))
        sys.exit(1)


if __name__ == "__main__": main()
