# Contributing

## 💬 Start on Discord

All coordination for this project happens on **The Quester's Rest**, the Dragon Quest IX Discord
server:

**https://discord.gg/DQIX**

Once you're in, join the **[DQI-haX: SWEs of the Starry Skies](https://discord.com/channels/655390550698098700/1266135635014582332)**
thread. That's where decompilation work is discussed, where functions get claimed so two people
don't match the same one, and where to ask for help when a function refuses to match.

---

## 🧭 Before you write code

1. Get a working build first. Follow the setup and build steps in [README.md](README.md) and make
   sure `ninja` succeeds on a clean checkout.
2. Read [Decompiling.md](Decompiling.md). It covers the Ghidra and dsd setup, how to reference
   symbols that haven't been decompiled yet, and how to add a new source file to `delinks.txt` and
   `symbols.txt`.
3. Say in the DQI-haX thread which function or file you're taking, so effort isn't duplicated.

---

## ✅ What gets merged

> [!Important]
> Submitted code must assemble to the **same bytes** as the original release. The build verifies
> every module against the ROM and will fail if your code doesn't match.

- Run `ninja` before opening a pull request and make sure it passes.
- Rename symbols in `symbols.txt` to match the names used in your decompiled code.
- Mark a file `complete` in `delinks.txt` only when every function in its address range matches.
- Keep one logical change per pull request; a batch of matched functions in the same module is fine,
  unrelated refactors in the same PR are not.

If a function is close but not byte-exact, it's still worth sharing. Post the
[decomp.me](https://decomp.me) scratch in the DQI-haX thread rather than opening a pull request, and
someone can pick up the remaining difference with you.

---

## 🐛 Issues and questions

Bug reports about the build or the tooling belong in GitHub issues. Questions about a specific
function, an idiom that won't reproduce, or how the game works are better in the DQI-haX thread,
where you'll get an answer faster.
