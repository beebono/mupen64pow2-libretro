# Mupen64Plus-pow2

A libretro N64 core built from Mupen64Plus-Next, reworked around the [mupen64plus-pow2](https://github.com/beebono/mupen64plus-pow2) stack for aarch64 handhelds. The output is `mupen64plus_pow2_libretro.so`.

What changed from Mupen64Plus-Next:

- **Core**: [mupen64plus-core-dento](https://github.com/beebono/mupen64plus-core-dento) (`libretro` branch: the Mupen64Plus-Next libretro patches on top of dento) replaces the libretro mupen64plus-core fork.
- **Plugins**: only [foir](https://github.com/beebono/mupen64plus-rdp-foir) (software LLE RDP, NEON on aarch64) and [arMLEt](https://github.com/beebono/mupen64plus-rsp-armlet) (RSP: rsp-hle + parallel-rsp + native graphics microcode). GLideN64, angrylion, parallel-rdp, rsp-hle, cxd4 and parallel-rsp are gone, and with them all GL / Vulkan rendering.
- **Vendoring**: the core and both plugins are git-subrepos with unmodified plugin sources; the libretro glue for the plugins is in `custom/pow2/` (symbol prefixes, core API lookups, foir's video output, options).
- **Platforms**: Linux x86_64 / aarch64 and Android arm64-v8a / x86_64.

The Mupen64Plus-Next README is kept below.

---

# Mupen64Plus-Next

Mupen64Plus-Next is a N64 emulation library for the [libretro API](http://www.libretro.com/), based on Mupen64Plus (see below).

It is also the successor of the old Mupen64Plus libretro core.

> You can *always* rely on it to give you an excellent Majora's Mask experience. Seriously.

#### How is this different from any N64 libretro-core, ever?

Due to the amount of libraries that are used and are in regular need of maintenance, I have strict rules about adding dependencies.  
This allows for easy maintenance, so available time can be spent on useful improvements and lowers the burden.  
By default the experience will be very simliar to the N64 emulators you know and love with *a lot extra*.

> **Sidenote:**  
While I accept pretty much every reasonable contribution, hacks must not impact behavior by default, unless justified.  
If you need to add a dependency, please consult me first.  
Force-pushes on all branches but `develop` and `master` are fair game.
`master` has the best stability memes, if that's your *thing*.

#### Used Technologies

The following projects have been incorporated into this repository:

- [mupen64plus](https://github.com/mupen64plus/mupen64plus-core)
- [GLideN64](https://github.com/gonetz/GLideN64)
- [cxd4](https://github.com/cxd4/rsp)
- [parallel-rsp](https://github.com/Themaister/parallel-rsp)
- [angrylion-rdp-plus](https://github.com/ata4/angrylion-rdp-plus) (Currently based on it's [ParaLLel](https://github.com/libretro/parallel-n64/) variant)

#### Acknowledgments

A special thanks to:

- The Mupen64Plus Team, especially Gillou68310
- gonetz and those that have worked on GLideN64, especially fzurita
- The Authors of cxd4 and angrylion-rdp-plus (ata4)
- themaister for parallel-rsp and parallel-rdp (including the Vulkan integration)
- Everyone in the libretro Team


\- m4xw
