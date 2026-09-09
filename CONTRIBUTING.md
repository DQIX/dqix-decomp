# Contributing

## 💬 Start on Discord

We frequently discuss this project in the Dragon Quest IX Discord server (https://discord.gg/DQIX) and as such we recommend joining to stay up to date. Once you're in, navigate to the **[DQI-haX: SWEs of the Starry Skies](https://discord.com/channels/655390550698098700/1266135635014582332)** thread - here you can discuss ongoing decompilation work, coordinate functions to work on to avoid duplicate work and get help with byte-matching any functions you're struggling to pin down.

---

## 🧭 Before you write code

1. Get a working build first. Follow the setup and build steps in [README.md](README.md) and make
   sure `ninja` succeeds on a clean checkout.
2. Read [Decompiling.md](Decompiling.md). It covers the Ghidra and dsd setup, how to reference
   symbols that haven't been decompiled yet, and how to add a new source file to `delinks.txt` and
   `symbols.txt`.
3. If you're planning to work on anything large, it would be good to mention this in the DQi-haX thread so others don't end up working on the exact same thing.

---

## ✅ What gets merged

> [!Important]
> Submitted code must assemble to the **same bytes** as the original release. The build verifies
> every module against the ROM and will fail if your code doesn't match.

- Run `ninja` before opening a pull request and make sure it passes.
- Tick both boxes in the pull request checklist. A check enforces them and names whichever is
  still unticked; edit the description and it re-runs.
- Rename symbols in `symbols.txt` to match the names used in your decompiled code.
- Mark a file `complete` in `delinks.txt` only when every function in its address range matches.
- Keep one logical change per pull request; a batch of matched functions in the same source file is fine,
  unrelated refactors in the same PR are not.

In addition to getting a byte match, you should ideally be fairly confident of what any decompiled code's purpose is, and functions/variables should be named in a way that reflects this purpose. Sometimes when decompiling a large class/struct, you might have some member variables that are unknown - this is fine, but try to match as much as possible before merging (feel free to ask for a second pair of eyes if you have any doubts!)

As a rule of thumb, you should aim for source file boundaries to match what they'd look like in a real codebase, i.e. with functions grouped by purpose. For example, if decompiling a class you should generally aim to put all of its member functions in one file. (This might not be possible if, for example, the functions are in separate parts of the binary, but we expect this to be a rare occurrence). With very large objects it's unreasonable to expect all member functions to be decompiled at once, so it's okay to temporarily make separate files for different parts as long as these are reasonably coherent. We may opt to put such partial progress on a separate branch to keep the main branch tidy.

If a function is close but not byte-exact, it's still worth sharing. Post the
[decomp.me](https://decomp.me) scratch in the DQI-haX thread rather than opening a pull request, and
someone can pick up the remaining difference with you.

---

## 🐛 Issues and questions

Bug reports about the build or the tooling belong in GitHub issues. Questions about a specific
function, an idiom that won't reproduce, or how the game works are better in the DQI-haX thread,
where you'll get an answer faster.
