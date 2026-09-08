#pragma once

// On-disk layout of the map data Zone3D reads between 0x0201383c and
// 0x02015134 (usa).
//
// Nothing here is a struct yet - the loader functions that would give these
// fields their runtime types are not decompiled. This is a record of what the
// files contain, so whoever writes those functions is not guessing at offsets.
//
// Provenance: the layouts below were established by parsing the retail files
// directly, independently of this disassembly. Where the disassembly confirms
// one of them the confirming function is named. Anything not marked confirmed
// is from file parsing alone and has not yet been checked against the code.

// ---------------------------------------------------------------------------
// data/map - 1386 files
// ---------------------------------------------------------------------------
//
//     .ambl  681    .amdj  669    .chr  18    .mse  12    .bin  4    .gp2  2
//
// The .ambl and .amdj sets run in parallel, roughly one pair per map.
//
// Both are containers whose header magic is "ARC", holding a single member
// that carries the same stem with the extension .bmbl or .bmdj respectively.
//
// CONFIRMED by disassembly: Zone3D::LoadMapAMBL builds the .ambl path and
// hands off to Zone3D::UnpackMapAMBL, which is the only function besides
// Zone3D::UnpackMapAMDJ to reference the "ARC" magic at 0x020ef13a;
// UnpackMapAMBL references ".bmbl" (0x020ef145) and UnpackMapAMDJ references
// ".bmdj" (0x020ef199). So the outer/inner extension pairing is established
// from both directions.

// ---------------------------------------------------------------------------
// data/map/maplist9.bin - the index of every map
// ---------------------------------------------------------------------------
//
// Loaded by Zone3D::SwitchZone, the top-level map load entry point - called
// from overlays 0, 1, 17 and 25.
//
// A tagged record table. Two tags matter:
//
//     0x66   the number of map entries
//     0x67   one record per map, carrying 22 values
//
// Four of the 22 values are byte offsets into the file's string table:
//
//     slot 4    region name
//     slot 6    map code - also the stem of the map's archive, so "M01" means
//               M01.ambl / M01.amdj. This is the link from a place to its files.
//     slot 7    the name used while the map was authored
//     slot 13   a second code, generally not this map's own
//
// CONFIRMED by disassembly: the handler table at data_020ef2c0 carries exactly
// four {tag, handler} pairs - 0x66 and 0x67 as above, plus 0x68 and 0x69, both
// of which this reader ignores (their handlers are `return 1`).
//
// The 0x67 handler (MapList_Opcode_67) runs once per entry and reads 16
// slots. Slot 0 is the zone ID and is the filter: entries for other zones are
// skipped before anything is read. The types below are the game's own, taken
// from which accessor it calls:
//
//     slot  0 1 3 6 8 9 10 12   int
//     slot  2 4 5 7 11          string (NULL when unset)
//     slot  13 14 15            float
//     slot  16+                 never read here
//
// Slot 11 is the string the reader keeps: it is strcpy'd into
// MapListInfo::maybeModelName. Slots 13, 14 and 15 are each multiplied by
// 4096.0f and truncated - the DS's 20.12 fixed point - and stored in
// MapListInfo::worldRotation (narrowed to 16 bits, after passing through
// func_02030f30), unknown_38 and unknown_3c.
//
// INFERRED - the slot numbering here is 2 lower than the numbering used by the
// file-parsing work this header quotes above. Under that shift every string
// slot that work identified lines up with a string slot the game reads:
//
//     its slot 4  (region)      -> slot 2   string
//     its slot 6  (map code)    -> slot 4   string
//     its slot 7  (author name) -> slot 5   string
//     its slot 13 (second code) -> slot 11  string, the one kept
//
// Four of four agreeing on one constant offset is good evidence, and it makes
// the handler see a pointer two values into the record - past what would be a
// two-value header. It also predicts one string that work did not list: slot 7
// here, its slot 9. That prediction is unverified and is the way to check the
// shift.
//
// If the shift holds, three of the eighteen unestablished values are settled:
// its slots 15, 16 and 17 are floats, carried as 20.12 fixed point.
//
// Trap: an unset field holds zero, and zero is also a valid string-table
// offset - it addresses the build stamp at the start of the table. A reader
// that resolves every offset produces a table in which every blank field looks
// like a date rather than an omission.
//
// Counts observed: 1010 entries, all carrying a code; 872 distinct codes, of
// which 667 name an archive that actually ships. The rest are development maps
// the table kept an entry for.

// ---------------------------------------------------------------------------
// .bmdj - the list of what a map is built from
// ---------------------------------------------------------------------------
//
// A map's archive holds a dozen loose files with no index between them. The
// .bmdj is that index, and is itself a tagged record table:
//
//     0x6A   the number of resources
//     0x6C   one per resource: position, a byte offset into the string table,
//            and two values that are not established
//     0x6F   one per resource, same order: where the map places it
//
// As with maplist9.bin the string is addressed by byte offset, not by index -
// counting names instead resolves the first correctly and drifts after it.
//
// The names are authoring names ("C01M0300.imd"). A single authored .imd
// compiles to several built files that all keep its stem, so a stem does not
// identify one file: C01M0300 is both a model and its texture animation.
// Resolving a resource to the first file matching its stem silently drops a
// map's main geometry - the map still assembles, without most of itself.
//
// Placement records (0x6F) carry 14 values. Three are established:
//
//     3, 4, 5   translation, as floats
//     6         the slot of the resource this one attaches to, or 0xFFFFFFFF
//
// A map piece is authored at its own origin and moved into place, so a piece
// drawn without its placement lands at the map's centre rather than where it
// belongs.

extern "C"
{

// usa: func_02014d18
// Names a member inside an already-open archive: "ARC:/" plus the stem, with
// the extension replaced.
void BuildArcMemberPath(const char* stem, const char* extension, char* path);

}
