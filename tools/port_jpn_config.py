#!/usr/bin/env python3

import argparse
from bisect import bisect_right
from collections import Counter, defaultdict
from pathlib import Path
import re
import struct
import sys

from region_port import (DELINK_START, block_name, branch_target, delink_blocks, hex_address, is_live, is_local,
                         live_ranges, read_raw, relocations, sync_delinks, thumb_branch_target, with_local)


parser = argparse.ArgumentParser(description="Ports matched files from config/usa to config/jpn")
parser.add_argument("--usa", type=Path, default=Path("config/usa"), help="USA config directory")
parser.add_argument("--jpn", type=Path, default=Path("config/jpn"), help="JPN config directory")
parser.add_argument("--extract", type=Path, default=Path("extract/jpn"), help="Extracted JPN ROM directory")
parser.add_argument("--sync", action="store_true",
                    help="Bring the USA matched files config/jpn lacks into it, keeping its own work")
parser.add_argument("--dry-run", action="store_true", help="With --sync, report without writing")
args = parser.parse_args()

SYMBOL = re.compile(r"^(\S+) kind:(\w+)(?:\(([^)]*)\))? addr:(0x[0-9a-f]+)")
SIZE = re.compile(r"size=(0x[0-9a-f]+)")
AUTO_NAME = re.compile(r"^(?:(?:func|data|bss)_(?:ov\d+_)?[0-9a-f]{8}|\.L_[0-9a-f]{8})$")
SOURCE_NAME = re.compile(r"\b(?:func|data|bss)_\w+|\b_Z\w+")
JPN_BRANCH = re.compile(r"#\s*(?:el)?if\s+defined\s*\(\s*jpn\s*\)(.*?)#\s*(?:endif|else|elif)", re.S)
DEFINE = re.compile(r"^\s*#\s*define\s+(\S+)\s+(\S+)\s*$", re.M)
TEXT_RANGE = re.compile(r"^\s*\.text\s+start:(0x[0-9a-f]+)\s+end:(0x[0-9a-f]+)")
POINTER_LOW, POINTER_HIGH = 0x01ff8000, 0x02800000


class Symbol:
    def __init__(self, name, kind, attrs, address, index, line):
        self.name = name
        self.kind = kind
        self.address = address
        self.index = index
        self.line = line
        size = SIZE.search(attrs or "")
        self.size = int(size[1], 16) if size else 0
        self.thumb = (attrs or "").startswith("thumb")
        self.place = (address, kind == "function" and self.thumb)


def parse_symbol(line, index=-1):
    match = SYMBOL.match(line)
    return Symbol(match[1], match[2], match[3], int(match[4], 16), index, line) if match else None


class Module:
    def __init__(self, root: Path, extract: Path, rel: Path):
        self.dir = root / "arm9" / rel
        self.name = rel.name if rel.parts else "main"
        if self.name == "main":
            binary = extract / "arm9" / "arm9.bin"
        elif self.name in ("itcm", "dtcm"):
            binary = extract / "arm9" / f"{self.name}.bin"
        else:
            binary = extract / "arm9_overlays" / f"{self.name}.bin"
        self.data = binary.read_bytes()
        delinks = read_raw(self.dir / "delinks.txt")
        self.base = 0x02000000 if self.name == "main" else min(int(s, 16) for s in DELINK_START.findall(delinks))
        self.lines = read_raw(self.dir / "symbols.txt").splitlines(keepends=True)
        self.symbols = []
        for index, line in enumerate(self.lines):
            symbol = parse_symbol(line, index)
            if symbol:
                self.symbols.append(symbol)
        self.symbols.sort(key=lambda s: s.address)
        self.by_index = {s.index: s for s in self.symbols}
        self.addresses = [s.address for s in self.symbols]
        self.at = defaultdict(list)
        for symbol in self.symbols:
            self.at[symbol.address].append(symbol)
        self.relocs = {source: (kind, target, module) for source, kind, target, module in relocations(self.dir / "relocs.txt")}
        self.reloc_sources = sorted(self.relocs)

    def in_image(self, start, size):
        return 0 <= start - self.base and start - self.base + size <= len(self.data)

    def word(self, address):
        return struct.unpack_from("<I", self.data, address - self.base)[0]

    def halfword(self, address):
        return struct.unpack_from("<H", self.data, address - self.base)[0]

    def masked_offsets(self, start, size, thumb):
        offsets = set()
        first = bisect_right(self.reloc_sources, start - 4)
        for source in self.reloc_sources[first:]:
            if source >= start + size:
                break
            offsets.add(source - start)
        for address in range((start + 3) & ~3, start + size - 3, 4):
            value = self.word(address)
            if POINTER_LOW <= value < POINTER_HIGH or value & 0x0f000000 == 0x0b000000 or value & 0xfe000000 == 0xfa000000:
                offsets.add(address - start)
        if thumb:
            for address in range((start + 1) & ~1, start + size - 3, 2):
                if self.halfword(address) & 0xf800 == 0xf000 and self.halfword(address + 2) & 0xe800 == 0xe800:
                    offsets.add(address - start)
        return offsets

    def masked(self, start, size, offsets):
        raw = bytearray(self.data[start - self.base:start - self.base + size])
        for offset in offsets:
            for i in range(max(offset, 0), min(offset + 4, size)):
                raw[i] = 0
        return bytes(raw)

    def symbol_containing(self, address):
        i = bisect_right(self.addresses, address) - 1
        return self.symbols[i] if i >= 0 else None

    def has_thumb(self, start, end):
        i = bisect_right(self.addresses, start - 1)
        return any(s.thumb for s in self.symbols[i:] if s.address < end and s.kind == "function")


def module_paths(root: Path):
    arm9 = root / "arm9"
    paths = [Path()]
    paths += [Path("overlays") / d.name for d in sorted((arm9 / "overlays").iterdir()) if (d / "delinks.txt").is_file()]
    paths += [Path(t) for t in ("itcm", "dtcm") if (arm9 / t / "delinks.txt").is_file()]
    return paths


def overlay_group(name):
    match = re.fullmatch(r"overlays\(([\d,]+)\)", name)
    return tuple(f"ov{int(n):03d}" for n in match[1].split(",")) if match else None


def reloc_module(name):
    if name in ("main", "itcm", "dtcm"):
        return name
    match = re.fullmatch(r"overlay\((\d+)\)", name)
    return f"ov{int(match[1]):03d}" if match else None


def increasing(pairs):
    tails, links, best = [], [], []
    for index, (_, jpn, _) in enumerate(pairs):
        i = _lower_bound(tails, pairs, jpn)
        links.append(tails[i - 1] if i else -1)
        if i == len(tails):
            tails.append(index)
        else:
            tails[i] = index
    index = tails[-1] if tails else -1
    while index != -1:
        best.append(pairs[index])
        index = links[index]
    return best[::-1]


def _lower_bound(tails, pairs, jpn):
    low, high = 0, len(tails)
    while low < high:
        mid = (low + high) // 2
        if pairs[tails[mid]][1] < jpn:
            low = mid + 1
        else:
            high = mid
    return low


class AnchorMap:
    def __init__(self, anchors):
        self.anchors = anchors
        self.usa = [u for u, _, _ in anchors]

    def map(self, address):
        i = bisect_right(self.usa, address) - 1
        if i < 0:
            raise ValueError(address)
        usa, jpn, size = self.anchors[i]
        if address < usa + size:
            return jpn + (address - usa)
        if i + 1 < len(self.anchors):
            next_usa, next_jpn, _ = self.anchors[i + 1]
            if next_jpn - jpn == next_usa - usa:
                return jpn + (address - usa)
        raise ValueError(address)

    def map_end(self, end):
        return self.map(end - 1) + 1


class DataMap:
    def __init__(self, code: AnchorMap, usa: Module, pairs):
        self.code = code
        self.anchors = code.anchors
        self.usa = usa
        self.pairs = pairs

    def map(self, address):
        try:
            return self.code.map(address)
        except ValueError:
            symbol = self.usa.symbol_containing(address)
            if symbol is None or symbol.address not in self.pairs:
                raise
            return self.pairs[symbol.address] + (address - symbol.address)

    def map_end(self, end):
        return self.map(end - 1) + 1


def data_pairs(usa: Module, jpn: Module, code: AnchorMap):
    found = defaultdict(set)
    for source, (kind, target, module) in usa.relocs.items():
        if reloc_module(module) != usa.name:
            continue
        symbol = usa.symbol_containing(target)
        if symbol is None or symbol.kind not in ("data", "bss"):
            continue
        try:
            jpn_source = code.map(source)
        except ValueError:
            continue
        if jpn_source not in jpn.relocs:
            continue
        jpn_kind, jpn_target, jpn_module = jpn.relocs[jpn_source]
        jpn_symbol = jpn.symbol_containing(jpn_target)
        if jpn_kind != kind or reloc_module(jpn_module) != jpn.name or jpn_symbol is None \
                or jpn_symbol.kind != symbol.kind or target - symbol.address != jpn_target - jpn_symbol.address:
            continue
        found[symbol.address].add(jpn_symbol.address)
    return {usa_address: jpn_addresses.pop() for usa_address, jpn_addresses in found.items() if len(jpn_addresses) == 1}


def anchors_for(usa: Module, jpn: Module):
    def keys(module):
        out = defaultdict(list)
        for s in module.symbols:
            if s.kind != "function" or not s.size or not module.in_image(s.address, s.size):
                continue
            offsets = module.masked_offsets(s.address, s.size, s.thumb)
            out[(s.thumb, s.size, module.masked(s.address, s.size, offsets))].append(s.address)
        return out
    usa_keys, jpn_keys = keys(usa), keys(jpn)
    pairs = sorted((addresses[0], jpn_keys[key][0], key[1]) for key, addresses in usa_keys.items()
                   if len(addresses) == 1 and len(jpn_keys.get(key, ())) == 1)
    return increasing(pairs)


def reproducible(usa: Module, jpn: Module, usa_start, usa_end, jpn_start, jpn_end):
    size = usa_end - usa_start
    if jpn_end - jpn_start == size and usa_start - usa.base >= len(usa.data) and jpn_start - jpn.base >= len(jpn.data):
        return True
    if jpn_end - jpn_start != size or not usa.in_image(usa_start, size) or not jpn.in_image(jpn_start, size):
        return False
    thumb = usa.has_thumb(usa_start, usa_end)
    offsets = usa.masked_offsets(usa_start, size, thumb) | jpn.masked_offsets(jpn_start, size, thumb)
    return usa.masked(usa_start, size, offsets) == jpn.masked(jpn_start, size, offsets)


def text_sources(usa: Module):
    _, blocks = delink_blocks(read_raw(usa.dir / "delinks.txt"))
    sources = {}
    for block in blocks:
        if not is_live(block):
            continue
        texts = [(int(m[1], 16), int(m[2], 16)) for line in block[1:] if not line.lstrip().startswith("//")
                 for m in [TEXT_RANGE.search(line)] if m]
        if len(texts) == 1:
            sources[texts[0]] = block[0].strip().rstrip(":")
    return sources


def object_reproduces(jpn: Module, code: AnchorMap, sources, usa_start, usa_end, jpn_start, jpn_end):
    source = sources.get((usa_start, usa_end))
    if source is None or jpn_end - jpn_start != usa_end - usa_start:
        return False
    try:
        if code.map(usa_start) != jpn_start or code.map_end(usa_end) != jpn_end:
            return False
    except ValueError:
        return False
    if not any(s.kind == "function" and s.size == jpn_end - jpn_start for s in jpn.at.get(jpn_start, [])):
        return False
    built = jpn_object(source)
    return built is not None and built.reproduces(jpn, jpn_start, jpn_end)


def source_names(root: Path):
    defines = {}
    words = set()
    for folder in ("src", "include"):
        for path in (root / folder).rglob("*"):
            if path.suffix not in (".c", ".cpp", ".h", ".hpp", ".inc", ".s"):
                continue
            text = path.read_text(encoding="utf-8", errors="replace")
            words.update(SOURCE_NAME.findall(text))
            for branch in JPN_BRANCH.findall(text):
                for name, target in DEFINE.findall(branch):
                    defines[name] = target
    return defines, words


class Plan:
    def __init__(self, jpn_modules, defines, words, usa_names, usa_local):
        self.modules = jpn_modules
        self.usa_names = usa_names
        self.usa_local = usa_local
        self.defines = defines
        self.targets = set(defines.values())
        self.words = words
        self.names = Counter(s.name for m in jpn_modules.values() for s in m.symbols)
        self.places = defaultdict(set)
        for module in jpn_modules.values():
            for symbol in module.symbols:
                self.places[symbol.name].add(symbol.place)
        self.renames = {}
        self.additions = defaultdict(list)
        self.labels = {}

    def current(self, module, symbol):
        return self.renames.get((module, symbol.index), symbol.name)

    def located(self, name, changes, additions):
        found = set(self.places.get(name, ()))
        for (module, index), new in changes.items():
            symbol = self.modules[module].by_index[index]
            if self.current(module, symbol) == name:
                found.discard(symbol.place)
            if new == name:
                found.add(symbol.place)
        found.update(parse_symbol(line).place for _, line, added in additions if added == name)
        return found

    def resolve(self, usa_name, module, symbol, changes):
        have = changes.get((module, symbol.index)) or self.current(module, symbol)
        if have == usa_name:
            return None
        if usa_name in self.defines:
            return None if self.defines[usa_name] == have else f"{usa_name} is defined to a JPN name"
        if (module, symbol.index) in self.renames or (module, symbol.index) in changes:
            return "name already claimed"
        if not AUTO_NAME.match(have) and have in self.usa_names:
            return f"JPN names it {have}"
        if have in self.targets or have in self.words:
            return f"a source refers to {have}"
        if self.names[usa_name] or usa_name in changes.values():
            return f"{usa_name} is taken in JPN"
        changes[(module, symbol.index)] = usa_name
        return None

    def commit(self, changes, additions):
        for key, name in changes.items():
            module, index = key
            old = self.renames.get(key) or self.modules[module].lines[index].split(" ", 1)[0]
            self.names[old] -= 1
            self.names[name] += 1
            self.renames[key] = name
            place = self.modules[module].by_index[index].place
            self.places[old].discard(place)
            self.places[name].add(place)
        for module, line, name in additions:
            self.additions[module].append(line)
            self.names[name] += 1
            self.places[name].add(parse_symbol(line).place)


class ObjectFile:
    def __init__(self, path: Path):
        data = path.read_bytes()
        if data[:4] != b"\x7fELF":
            raise ValueError(path)
        shoff = struct.unpack_from("<I", data, 0x20)[0]
        entsize, count, names_index = struct.unpack_from("<HHH", data, 0x2e)
        sections = [struct.unpack_from("<10I", data, shoff + i * entsize) for i in range(count)]

        def string(table, offset):
            start = sections[table][4] + offset
            return data[start:data.index(b"\0", start)].decode("latin-1")

        names = [string(names_index, s[0]) for s in sections]
        texts = [i for i, s in enumerate(sections) if names[i] == ".text"]
        self.text, bases = None, {}
        if texts:
            text = bytearray()
            for i in texts:
                text.extend(bytes(-len(text) % max(sections[i][8], 1)))
                bases[i] = len(text)
                text.extend(data[sections[i][4]:sections[i][4] + sections[i][5]])
            self.text = bytes(text)
        self.align = max(sections[texts[0]][8], 1) if texts else 1
        self.other = [names[i] for i, s in enumerate(sections) if s[2] & 2 and names[i] != ".text" and s[5]]
        self.sizes = {names[i]: s[5] for i, s in enumerate(sections) if s[2] & 2 and s[5]}
        self.relocations = []
        self.defined, self.undefined = set(), set()
        for i, section in enumerate(sections):
            if section[1] in (4, 9) and section[7] in texts:
                step = 12 if section[1] == 4 else 8
                for o in range(section[4], section[4] + section[5], step):
                    offset, info = struct.unpack_from("<II", data, o)
                    addend = struct.unpack_from("<i", data, o + 8)[0] if step == 12 else None
                    entry = sections[section[6]][4] + (info >> 8) * 16
                    name_offset, value, _, symbol_info, _, index = struct.unpack_from("<IIIBBH", data, entry)
                    if index in texts:
                        name = None
                    elif index < len(names) and names[index] == ".bss" and symbol_info & 0xf == 3:
                        name = ".bss"
                    else:
                        name = string(sections[section[6]][6], name_offset)
                    self.relocations.append((offset + bases[section[7]], info & 0xff, name,
                                             value + bases[index] if index in texts else 0, addend))
            if section[1] != 2:
                continue
            for offset in range(section[4] + 16, section[4] + section[5], 16):
                name_offset, _, _, info, _, index = struct.unpack_from("<IIIBBH", data, offset)
                if not name_offset or info & 0xf in (3, 4):
                    continue
                name = string(section[6], name_offset)
                if name.startswith(".") and not name.startswith(".L_") or "@" in name:
                    continue
                if index == 0:
                    self.undefined.add(name)
                elif info >> 4 in (1, 2):
                    self.defined.add(name)

    def reproduces(self, module: Module, start: int, end: int) -> bool:
        if self.text is None or len(self.text) != end - start or start % self.align or not module.in_image(start, end - start):
            return False
        built = bytearray(self.text)
        rom = bytearray(module.data[start - module.base:end - module.base])
        for offset, *_ in self.relocations:
            built[offset:offset + 4] = rom[offset:offset + 4] = b"\0\0\0\0"
        return built == rom


def jpn_object(source: str):
    path = Path(source)
    built = Path("build") / "jpn" / path.with_suffix(".o")
    if not path.is_file() or not built.is_file() or built.stat().st_mtime < path.stat().st_mtime:
        return None
    try:
        return ObjectFile(built)
    except (ValueError, IndexError, struct.error):
        return None


ADDRESS = re.compile(r"addr:0x[0-9a-f]+")
R_ARM_PC24, R_ARM_ABS32, R_ARM_THM_CALL = 1, 2, 10


def lands(module: Module, kind, source, target, thumb):
    word = module.word(source)
    if kind == R_ARM_ABS32:
        return word == target | thumb
    if kind == R_ARM_PC24:
        return branch_target(word, source) == (target + 8) & ~1
    if kind == R_ARM_THM_CALL:
        return thumb_branch_target(word, source) == (target + 4) & ~1
    return False


def accept_for(plan: Plan, usa_modules, jpn_modules, usa: Module, jpn: Module, mapper: AnchorMap):
    def group_lands(group, target, jpn_target):
        for overlay in group:
            usa_module, jpn_module = usa_modules.get(overlay), jpn_modules.get(overlay)
            if not usa_module or not jpn_module:
                continue
            usa_symbol = usa_module.symbol_containing(target)
            jpn_symbol = jpn_module.symbol_containing(jpn_target)
            if usa_symbol and jpn_symbol and usa_symbol.name == jpn_symbol.name \
                    and target - usa_symbol.address == jpn_target - jpn_symbol.address < max(usa_symbol.size, 1):
                return True
        return False

    def bind(usa_module, usa_address, jpn_module, jpn_address, changes, additions, may_add):
        for label in (False, True):
            refused = bind_kind(usa_module, usa_address, jpn_module, jpn_address, changes, additions, may_add, label)
            if refused:
                return refused
        return None

    def bind_kind(usa_module, usa_address, jpn_module, jpn_address, changes, additions, may_add, label):
        usa_here = [s for s in usa_module.at.get(usa_address, []) if (s.kind == "label") == label]
        jpn_here = [s for s in jpn_module.at.get(jpn_address, []) if (s.kind == "label") == label]
        if not usa_here:
            return None
        if not jpn_here and not may_add:
            return f"JPN has no symbol at {hex_address(jpn_address)}"
        if jpn_here and usa_here[0].kind == "function" and jpn_here[0].size and usa_here[0].size != jpn_here[0].size:
            return f"{usa_here[0].name} is {jpn_here[0].size:#x} bytes in JPN"
        have = {changes.get((jpn_module.name, s.index)) or plan.current(jpn_module.name, s) for s in jpn_here}
        pending = [s for s in usa_here if s.name not in have]
        if jpn_here and pending and not have & {s.name for s in usa_here}:
            refused = plan.resolve(pending[0].name, jpn_module.name, jpn_here[0], changes)
            if refused:
                return refused
            pending = pending[1:]
        for symbol in pending:
            if plan.names[symbol.name] or symbol.name in changes.values() or any(n == symbol.name for _, _, n in additions):
                return f"{symbol.name} is taken in JPN"
            additions.append((jpn_module.name, ADDRESS.sub(f"addr:{hex_address(jpn_address)}", symbol.line, 1), symbol.name))
        return None

    def accept(block, ranges, linear=None):
        layout = linear or mapper
        if any(not line.lstrip().startswith(("//", ".text", ".bss")) and line.strip() and line.strip() != "complete"
               for line in block[1:]):
            return "has sections other than .text and .bss"
        sections = [line.split()[0] for line in block[1:] if DELINK_START.search(line) and not line.lstrip().startswith("//")]
        text = [r for section, r in zip(sections, ranges) if section == ".text"]
        bss = [r for section, r in zip(sections, ranges) if section == ".bss"]
        built = jpn_object(block[0].strip().rstrip(":"))
        if built is None:
            return "has no fresh JPN object"
        unplaced = [name for name in built.other if not (name == ".bss" and len(bss) == 1)]
        if unplaced:
            return f"compiles {unplaced[0]} for JPN"
        if bss and (".bss" not in built.sizes or (built.sizes[".bss"] + 3) & ~3 != bss[0][1] - bss[0][0]):
            return "has a .bss range its JPN object does not fill"
        if len(text) != 1 or len(sections) != len(ranges) or not built.reproduces(jpn, text[0][0], text[0][1]):
            return "compiles to other bytes for JPN"
        changes = {}
        additions = []
        for jpn_start, jpn_end, usa_start, usa_end in ranges:
            everything = [s for s in usa.symbols[bisect_right(usa.addresses, usa_start - 1):] if s.address < usa_end]
            mapped = {(s.kind == "label", layout.map(s.address)) for s in everything}
            before = jpn.symbol_containing(jpn_start - 1)
            if before and before.size and before.address + before.size > jpn_start:
                return f"JPN's {before.name} runs into it"
            for symbol in jpn.symbols[bisect_right(jpn.addresses, jpn_start - 1):]:
                if symbol.address >= jpn_end:
                    break
                if (symbol.kind == "label", symbol.address) not in mapped:
                    return f"JPN has {symbol.name} inside it"
                if symbol.address + symbol.size > jpn_end:
                    return f"JPN's {symbol.name} runs past it"
            for address in sorted({s.address for s in everything}):
                refused = bind(usa, address, jpn, layout.map(address), changes, additions, True)
                if refused:
                    return refused
            first = bisect_right(usa.reloc_sources, usa_start - 1)
            for source in usa.reloc_sources[first:]:
                if source >= usa_end:
                    break
                kind, target, module = usa.relocs[source]
                jpn_source = layout.map(source)
                if jpn_source not in jpn.relocs:
                    return f"JPN has no relocation at {hex_address(jpn_source)}"
                jpn_kind, jpn_target, jpn_module = jpn.relocs[jpn_source]
                if kind != jpn_kind:
                    return f"relocation at {hex_address(source)} is {kind} in USA and {jpn_kind} in JPN"
                if module == "none" or jpn_module == "none":
                    if module != jpn_module or jpn.word(jpn_source) != usa.word(source):
                        return f"absolute word at {hex_address(source)} differs"
                    continue
                group = overlay_group(module)
                if group is not None or overlay_group(jpn_module) is not None:
                    if group != overlay_group(jpn_module):
                        return f"relocation at {hex_address(source)} targets {module} and {jpn_module}"
                    if not group_lands(group, target, jpn_target):
                        return f"relocation at {hex_address(source)} lands differently"
                    continue
                usa_target_module = usa_modules.get(reloc_module(module))
                jpn_target_module = jpn_modules.get(reloc_module(jpn_module))
                if not usa_target_module or not jpn_target_module or usa_target_module.name != jpn_target_module.name:
                    return f"relocation at {hex_address(source)} targets {module} and {jpn_module}"
                usa_symbol = usa_target_module.symbol_containing(target)
                jpn_symbol = jpn_target_module.symbol_containing(jpn_target)
                if not usa_symbol or not jpn_symbol or target - usa_symbol.address != jpn_target - jpn_symbol.address:
                    return f"relocation at {hex_address(source)} lands differently"
                refused = bind(usa_target_module, usa_symbol.address, jpn_target_module, jpn_symbol.address,
                               changes, additions, False)
                if refused:
                    return refused
        missing = sorted(n for n in built.defined | built.undefined if not plan.located(n, changes, additions))
        if missing:
            return f"needs {missing[0]}, which JPN lacks"
        start = text[0][0]
        for offset, kind, name, value, addend in built.relocations:
            if name is None:
                places = {(start + value, False)}
            elif name == ".bss":
                places = {(bss[0][0], False)} if bss else set()
            else:
                places = plan.located(name, changes, additions)
            if addend is None or not places or not all(lands(jpn, kind, start + offset, address + addend, thumb)
                                                       for address, thumb in places):
                return f"relocation at {hex_address(start + offset)} to {name or 'itself'} lands elsewhere in JPN"
        plan.commit(changes, additions)
        return None
    return accept


def relabel(plan: Plan, usa: Module, jpn: Module, mapper: AnchorMap):
    anchored = {j: (u, size) for u, j, size in mapper.anchors}
    functions = [s for s in jpn.symbols if s.kind == "function" and s.address in anchored]
    for symbol in jpn.symbols:
        if symbol.kind != "data":
            continue
        for function in functions:
            if function.address < symbol.address < function.address + function.size:
                usa_address = anchored[function.address][0] + (symbol.address - function.address)
                if any(s.kind == "label" for s in usa.at.get(usa_address, [])):
                    plan.labels[(jpn.name, symbol.index)] = "thumb" if function.thumb else "arm"


def write_symbols(plan: Plan):
    for name, module in plan.modules.items():
        renames = {index: new for (owner, index), new in plan.renames.items() if owner == name}
        labels = {index: isa for (owner, index), isa in plan.labels.items() if owner == name}
        additions = plan.additions.get(name, [])
        if not renames and not additions and not labels:
            continue
        lines = list(module.lines)
        for index, isa in labels.items():
            lines[index] = re.sub(r"kind:data(?:\([^)]*\))?", f"kind:label({isa})", lines[index], count=1)
        for index, new in renames.items():
            lines[index] = with_local(new + lines[index][len(lines[index].split(" ", 1)[0]):],
                                      (name, new) in plan.usa_local)
        newline = "\r\n" if lines and lines[-1].endswith("\r\n") else "\n"
        if lines and not lines[-1].endswith(("\n", "\r\n")):
            lines[-1] += newline
        lines += [line.rstrip("\r\n") + newline for line in additions]
        (module.dir / "symbols.txt").write_text("".join(lines), encoding="utf-8", newline="")


class Linear:
    def __init__(self, usa_start, jpn_start):
        self.delta = jpn_start - usa_start

    def map(self, address):
        return address + self.delta

    def map_end(self, end):
        return end + self.delta


def refresh(usa: Module, jpn: Module, accept):
    _, usa_blocks = delink_blocks(read_raw(usa.dir / "delinks.txt"))
    _, jpn_blocks = delink_blocks(read_raw(jpn.dir / "delinks.txt"))
    ported = {block_name(b): b for b in jpn_blocks if is_live(b)}
    for block in usa_blocks:
        if not is_live(block) or block_name(block) not in ported:
            continue
        usa_ranges, jpn_ranges = live_ranges(block), live_ranges(ported[block_name(block)])
        if len(usa_ranges) != 1 or len(jpn_ranges) != 1:
            continue
        (usa_start, usa_end), (jpn_start, jpn_end) = usa_ranges[0], jpn_ranges[0]
        if usa_end - usa_start == jpn_end - jpn_start:
            accept(block, [(jpn_start, jpn_end, usa_start, usa_end)], Linear(usa_start, jpn_start))


def sync():
    if not (args.extract / "arm9" / "arm9.bin").is_file() or not (args.jpn / "arm9").is_dir():
        sys.exit(f"--sync needs {args.jpn} and an extracted JPN ROM at {args.extract}")
    usa_extract = args.extract.parent / "usa"
    usa_modules, jpn_modules = {}, {}
    for rel in module_paths(args.usa):
        if (args.jpn / "arm9" / rel / "delinks.txt").is_file():
            usa = Module(args.usa, usa_extract, rel)
            usa_modules[usa.name] = usa
            jpn_modules[usa.name] = Module(args.jpn, args.extract, rel)
    defines, words = source_names(args.usa.parent.parent)
    plan = Plan(jpn_modules, defines, words, {s.name for m in usa_modules.values() for s in m.symbols},
                {(m.name, s.name) for m in usa_modules.values() for s in m.symbols if is_local(s.line)})
    total = 0
    reasons = Counter()
    mappers, accepts = {}, {}
    for name, usa in usa_modules.items():
        code = AnchorMap(anchors_for(usa, jpn_modules[name]))
        mappers[name] = DataMap(code, usa, data_pairs(usa, jpn_modules[name], code))
        accepts[name] = accept_for(plan, usa_modules, jpn_modules, usa, jpn_modules[name], mappers[name])
        refresh(usa, jpn_modules[name], accepts[name])
    for name, usa in usa_modules.items():
        jpn = jpn_modules[name]
        mapper = mappers[name]
        relabel(plan, usa, jpn, mapper)
        sources = text_sources(usa)
        added, notes = sync_delinks(
            usa.dir / "delinks.txt", jpn.dir / "delinks.txt", mapper,
            lambda us, ue, js, je: reproducible(usa, jpn, us, ue, js, je)
            or object_reproduces(jpn, mapper.code, sources, us, ue, js, je), "JPN", args.dry_run, accepts[name])
        total += added
        for note in notes:
            reasons[re.sub(r"0x[0-9a-f]+|\b\w*_?[0-9a-f]{8}\w*|\S+\.(?:cpp|c|s)\b", "#", note.split(" ", 1)[1])] += 1
        print(f"{jpn.dir}: {len(mapper.anchors)} anchors, {added} files, {len(notes)} left")
    if not args.dry_run:
        write_symbols(plan)
    print("Left as JPN ROM bytes, by reason:")
    for reason, count in reasons.most_common(20):
        print(f"  {count:6d}  {reason}")
    renamed = len(plan.renames) + len(plan.labels)
    added_names = sum(len(v) for v in plan.additions.values())
    print(f"{'Would bring' if args.dry_run else 'Brought'} {renamed + added_names} names and {total} files into {args.jpn}")


def main():
    if not args.sync:
        sys.exit("only --sync is supported: config/jpn is maintained upstream and extended from config/usa")
    sync()


if __name__ == "__main__": main()
