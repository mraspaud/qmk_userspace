# Cantor Pro MX — Shore lighting implementation (first hardware candidate)

This is a complete *keymap-side* implementation of `LIGHTING_SPEC.md` v0.4,
for the existing `42keebs/cantor_pro/v3/left` and `/right` QMK targets.
The user's combo/keymap code is preserved. The old animated HSV lighting
has been replaced with a separate Shore module.

**Status:** C compiler host-stub smoke test passes. **Not compiled against your
actual Vial/QMK fork or tested on the physical keyboard here.** Treat this as
an implementation candidate, not a verified UF2 binary. The exact RGB driver
and split-USB handling must be confirmed when compiling/flashing on your host.

## Install

Put these four files in your QMK userspace keymap directory:

`qmk_userspace/keyboards/42keebs/cantor_pro/keymaps/mraspaud/`

- `keymap.c` (replaces your current file)
- `config.h` (replaces your current file)
- `shore_lighting.h` (new)
- `shore_config.h` (new; all tunables in one place)

**Leave your current `rules.mk` unchanged.** It already works with the
Cantor v3 RGBLIGHT setup. Do not change the controller/PCB definitions.

The uploaded starting files are also preserved in `baseline/` inside this package.
Back up your known-good firmware and keymap before trying this:

```sh
cp keymap.c keymap.before-shore.c.bak
cp config.h config.before-shore.h.bak
```

Compile both targets **before flashing either**:

```sh
qmk compile -kb 42keebs/cantor_pro/v3/left -km mraspaud
qmk compile -kb 42keebs/cantor_pro/v3/right -km mraspaud
```

You previously used these per-physical-side UF2 flows (flash only when you
are ready and the relevant MCU is in UF2 mode):

```sh
qmk flash -kb 42keebs/cantor_pro/v3/left  -km mraspaud -bl uf2-split-left
qmk flash -kb 42keebs/cantor_pro/v3/right -km mraspaud -bl uf2-split-right
```

**Always flash physical left with `v3/left`, physical right with `v3/right`.**
The reversible PCB swaps WS2812 and split-serial MCU pins between halves.
Keep the TRRS connected only with power disconnected, as before.

## What it does

- 14 LEDs, seven per half, at actual KiCad positions; electrical order is not
  used for spatial sampling. The top LEDs sample the same field as underglow.
- `SHORE_HALF_GAP = 1.0f` keyboard-half widths by default.
- Directional sea-to-beach advance and asymmetric retreat, with smooth
  continuously changing speed, a slow global breath, and a nonzero baseline.
- 15.9-second *spatial* cold entrance: small arrival, retreat, bigger return,
  baseline establishment and handover to normal Shore.
- Persistent palettes: coder Kanagawa waveAqua2, Irish green/orange,
  Swedish blue/yellow, French blue/red.
- Interruptible 1.8-second OKLab colour transitions. Different brightness
  envelope for coder versus flags is also transitioned, not abruptly switched.
- Animated top LEDs: dim persistent-colour baseline, 200 ms emphasis fade and
  240 ms colour morph to Kanagawa oniViolet (symbols) or dragonBlue (NAV).
  `L_FRSYM` shares symbols indication; `L_TH` uses coder/base colouring.
- Both halves compute from `sync_timer_read32()`, plus a 250-ms master->slave
  RPC heartbeat conveying USB-active state and animation start epoch.
- Buffered 7-pixel writes followed by **one WS2812 flush per half per frame**.
  Refresh is capped at 29.41 fps using a 34-ms interval; elapsed time, not
  frame count, controls the animations.
- No host service, LEDs dark when USB is not configured, no shutdown animation,
  and no experimental breathing during genuine USB suspend.

## Tunable parameters

Change only `shore_config.h` for initial visual tuning:

| Setting | Default | Meaning |
|---|---:|---|
| `SHORE_HALF_GAP` | 1.0 | Gap in units of one PCB width |
| `SHORE_SEA_ON_LEFT` | 1 | 0 reverses sea/beach |
| `SHORE_WAVE_HALF_WIDTH` | 1.02 | Spatial width in same units |
| `SHORE_WAVE_TILT` | 0.065 | Mild diagonal front |
| `SHORE_CYCLE_MS` | 42000 | Advance + retreat period |
| `SHORE_INCOMING_FRACTION` | 0.57 | Fraction of cycle devoted to advance |
| `SHORE_WAVE_STRENGTH` | 0.83 | Moving pool intensity |
| `SHORE_BREATH_STRENGTH` | 0.085 | Ambient global breathing |
| `SHORE_LANGUAGE_MIN/MAX` | 0.16 / 0.72 | Linear-light language brightness envelope |
| `SHORE_CODER_MIN/MAX` | 0.040 / 0.20 | Dim coder envelope |
| `SHORE_TOP_AMBIENT_MIN/MAX` | 0.003 / 0.011 | Top LEDs in normal typing |
| `SHORE_TOP_ACTIVE_MIN/MAX` | 0.036 / 0.115 | Top LEDs in NAV/Symbols |
| `SHORE_TOP_OUTPUT_CAP` | 0.13 | Direct-view safety cap |
| `SHORE_PALETTE_MORPH_MS` | 1800 | Persistent colour + envelope transition |
| `SHORE_TOP_EMPHASIS_MS` | 200 | Top brightness transition |
| `SHORE_TOP_COLOUR_MORPH_MS` | 240 | Top colour transition |
| `SHORE_FRAME_MS` | 34 | Max ~29.4 fps; don't set below 34 |

Brightness parameters here are **multipliers in linear light**, not HSV V or
perceived-lightness percentages. They need physical tuning with bare LEDs.

## First hardware verification

1. Confirm both halves type normally and either side can be USB primary.
2. With USB plugged into an awake/configured host, both halves stay initially
   dark and enter from the configured sea side. Watch the wave traverse the
   inter-half gap instead of jumping instantly.
3. After ~16 s, both halves retain dimly illuminated LEDs even outside the
   brighter moving swell. Verify the top LEDs are faintly animated.
4. Switch EN -> SE -> FR -> BASE mid-swell. Motion must not restart and colour
   changes should be smooth, including interruption mid-morph.
5. Hold/release `L_NUMSYM` and `L_NAV`: top LED stays wave-synchronised, gains
   emphasis over ~200 ms, then returns to ambient colour/brightness.
6. Test USB on *each physical side*. Confirm neither half reverses geometry.
7. Plug into a sleeping-but-powered laptop: if it is not USB configured, the
   keyboard should remain dark. A host retaining configured USB service while
   asleep is **not detectable as asleep from USB configured state alone**.
8. Test an actual bus suspend if available; see caveat below.

## Important limitations / fork checks

- `rgblight_driver` and its `.set_color`/`.flush` interface are used as present
  in modern QMK. If your specific Vial fork differs, the build will identify
  the mismatch. This package was **not compiled in your exact QMK tree**.
- The master sends USB/epoch over a small custom transaction (`SHORE_SYNC`);
  the secondary goes dark if no heartbeat arrives for 1.2 s. USB suspend tries
  to send a final off packet but **cannot guarantee an immediate secondary-off
  response** if split transport is already suspended. Confirm behaviour.
- **USB suspend power compliance is not guaranteed by this firmware**: two
  RP2040s and fourteen WS2812s may exceed USB suspend limits even when dark.
  It needs hardware current measurement and/or a different low-power design.
  The optional sleeping-laptop breathing effect is intentionally absent.
- `NO_USB_STARTUP_CHECK` and your `USB_SUSPEND_ENABLE = no` rule were preserved
  because they were part of the known-working keyboard. If a sleeping host
  keeps the device *configured*, USB state cannot distinguish it from awake.
- The default top LEDs are direct-view; if they're uncomfortable, lower the
  `SHORE_TOP_*` values and cap before modifying the wave code.
- Until the translucent case exists, these are discrete LEDs, not a calibrated
  diffuse optical surface. Future optics live behind the geometry/field layer.

## Minimal test (without QMK toolchain)

The `tests/` directory contains mocked versions of the small QMK API used by
this module. Compile on a PC with:

```sh
gcc -std=gnu11 -Wall -Wextra -Werror -Wno-unused-function \
  -Itests/stubs tests/host_test.c -o /tmp/shore_host_test
/tmp/shore_host_test
```

It checks unconfigured-USB darkness, arrival, nonzero normal lighting,
frame throttling, top-layer response and going dark on suspend. It does NOT
simulate serial transport timings or verify the real QMK fork.

## Cold boot: split master election (`split_role.h`)

QMK's `SPLIT_USB_DETECT` gives each half `SPLIT_USB_TIMEOUT` to see an
enumerated USB host. On a slow cold boot the USB half misses it, both halves
become secondary, and the keyboard is dead until replugged. A long timeout
fixes that but makes the secondary always wait the whole timeout.

`split_role.h` replaces QMK's weak `is_keyboard_master_impl()`. Each half waits
(without timeout by default) until either its USB is enumerated (it is master)
or it sees the master's serial traffic on the split line (it is secondary, and
starts within a few ms of the master). No reboots, no extra protocol.
`SPLIT_USB_TIMEOUT` and `SPLIT_WATCHDOG_ENABLE` are not needed with it.

Host test of the election logic:

```sh
gcc -std=gnu11 -Wall -Wextra -Werror -I. -Itests/stubs \
  tests/split_role_test.c -o /tmp/split_role_test
/tmp/split_role_test
```
