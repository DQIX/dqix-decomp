from pathlib import Path
import re


SYMBOL_ADDR = re.compile(r"addr:(0x[0-9a-f]+)")
SYMBOL_SIZE = re.compile(r"size=(0x[0-9a-f]+)")
DELINK_START = re.compile(r"start:(0x[0-9a-f]+)")
DELINK_END = re.compile(r"end:(0x[0-9a-f]+)")
RELOC = re.compile(r"from:(0x[0-9a-f]+) kind:(\S+) to:(0x[0-9a-f]+)(?: add:(0x[0-9a-f]+))? module:(\S+)")


def read_raw(path: Path) -> str:
    with path.open("r", encoding="utf-8", newline="") as file:
        return file.read()


def hex_address(address: int) -> str:
    return f"{address:#010x}"


def delink_blocks(text: str):
    lines = text.splitlines(keepends=True)
    header = []
    blocks = []
    current = None
    for line in lines:
        if line.endswith(":\n") or line.endswith(":\r\n"):
            if current:
                blocks.append(current)
            current = [line]
        elif current is None:
            header.append(line)
        else:
            current.append(line)
    if current:
        blocks.append(current)
    return header, blocks


def block_ranges(block: list[str]):
    ranges = []
    for line in block:
        start = DELINK_START.search(line)
        end = DELINK_END.search(line)
        if start and end:
            ranges.append((int(start[1], 16), int(end[1], 16)))
    return ranges


def port_delink(line: str, mapper) -> str:
    line = DELINK_START.sub(lambda m: f"start:{hex_address(mapper.map(int(m[1], 16)))}", line)
    return DELINK_END.sub(lambda m: f"end:{hex_address(mapper.map_end(int(m[1], 16)))}", line)


class SameAddress:
    def map(self, address: int) -> int:
        return address

    def map_end(self, end: int) -> int:
        return end


def relocations(path: Path) -> list[tuple[int, str, int, str]]:
    if not path.is_file():
        return []
    out = []
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        match = RELOC.search(line)
        if match:
            out.append((int(match[1], 16), match[2], int(match[3], 16) + int(match[4] or "0", 16), match[5]))
    return out


def branch_target(instruction: int, source: int) -> int | None:
    offset = instruction & 0xffffff
    if offset & 0x800000:
        offset -= 1 << 24
    if instruction >> 25 == 0x7d:
        return source + 8 + offset * 4 + ((instruction >> 24) & 1) * 2
    if instruction & 0x0e000000 == 0x0a000000:
        return source + 8 + offset * 4
    return None


def thumb_branch_target(instructions: int, source: int) -> int | None:
    high, low = instructions & 0xffff, instructions >> 16
    if high & 0xf800 != 0xf000:
        return None
    offset = ((high & 0x7ff) << 12) | ((low & 0x7ff) << 1)
    if offset & 0x400000:
        offset -= 1 << 23
    if low & 0xf800 == 0xf800:
        return source + 4 + offset
    if low & 0xf800 == 0xe800:
        return (source + 4 + offset) & ~3
    return None


def reloc_sources(path: Path) -> list[int]:
    return [source for source, _, _, _ in relocations(path)]


def is_local(line: str) -> bool:
    return "local" in line.split()[1:]


def with_local(line: str, local: bool) -> str:
    if is_local(line) == local:
        return line
    body = line.rstrip("\r\n")
    words = [word for word in body.split(" ") if word != "local"] + (["local"] if local else [])
    return " ".join(words) + line[len(body):]


def symbol_lines(path: Path):
    lines = read_raw(path).splitlines(keepends=True)
    by_address = {}
    for index, line in enumerate(lines):
        match = SYMBOL_ADDR.search(line)
        if match:
            by_address.setdefault(int(match[1], 16), []).append(index)
    return lines, by_address


def block_name(block: list[str]) -> str:
    return block[0].strip()


def is_live(block: list[str]) -> bool:
    return not block[0].lstrip().startswith("//")


def live_ranges(block: list[str]):
    return block_ranges([line for line in block if not line.lstrip().startswith("//")])


def sync_delinks(usa_path: Path, region_path: Path, mapper, reproducible, region: str, dry_run: bool,
                 accept=None) -> tuple[int, list[str]]:
    _, usa_blocks = delink_blocks(read_raw(usa_path))
    usa_blocks = [b for b in usa_blocks if is_live(b)]
    text = read_raw(region_path)
    header, blocks = delink_blocks(text)
    usa_names = {block_name(b) for b in usa_blocks}
    names = {block_name(b) for b in blocks if is_live(b)}
    only_here = names - usa_names
    owned = [(block_name(b), start, end) for b in blocks if is_live(b) for start, end in live_ranges(b)]
    added = []
    left = []
    replaced = set()
    for block in usa_blocks:
        name = block_name(block)
        if name in names:
            continue
        try:
            ranges = [(mapper.map(start), mapper.map_end(end), start, end) for start, end in live_ranges(block)]
        except ValueError:
            left.append(f"{name} has no {region} address")
            continue
        if any(start & 3 or end & 3 for start, end, _, _ in ranges):
            left.append(f"{name} is unaligned in {region}")
            continue
        if not all(reproducible(usa_start, usa_end, start, end) for start, end, usa_start, usa_end in ranges):
            left.append(f"{name} differs in {region}")
            continue
        overlapping = {owner for owner, start_here, end_here in owned
                       for start, end, _, _ in ranges if start_here < end and start < end_here}
        if overlapping - only_here:
            left.append(f"{name} overlaps {sorted(overlapping - only_here)}")
            continue
        refused = accept(block, ranges) if accept else None
        if refused:
            left.append(f"{name} {refused}")
            continue
        replaced |= overlapping
        added.append("".join(port_delink(line, mapper) for line in block))
    if (added or replaced) and not dry_run:
        newline = "\r\n" if "\r\n" in text else "\n"
        kept = "".join("".join(b) for b in blocks if block_name(b) not in replaced)
        body = ("".join(header) + kept).rstrip("\r\n") + newline
        for block in added:
            body += newline + block.rstrip("\r\n").replace("\r\n", "\n").replace("\n", newline) + newline
        region_path.write_text(body, encoding="utf-8", newline="")
    return len(added), [f"{name} replaced as a renamed file" for name in sorted(replaced)] + left
