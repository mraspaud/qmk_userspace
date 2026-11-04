# Cantor Pro MX — Ambient Lighting Specification

**Status:** Draft v0.4 — for review, not yet approved for implementation  
**Hardware:** 42. Keebs Cantor Pro MX v3, reversible PCB, two RP2040-based halves  
**Primary design:** Shore (organic sea-to-beach illumination)  
**Future interchangeable effects:** Tide and Living Light

## 1. Intent and design principles

Create restrained, organic ambient lighting that makes the keyboard feel like a single, softly illuminated object. The reference is the slow, apparently living pulse of Apple's historic sleep indicator, **not** decorative RGB animation.

The lighting shall:

- Refresh at **no more than 30 frames per second** in normal operation, without compromising key scanning or split transport.

- Feel calm and continuous, with gentle, non-mechanical variation in speed.
- Preserve the identity of the selected persistent layer through its colour palette.
- Form one spatial animation spanning both physically separated halves, even if their colours differ.
- Treat the physical gap as space that a wave must travel across, rather than jumping instantaneously between halves.
- Keep the animation phase continuous through layer changes; colour changes must morph without interrupting motion.
- Include **all fourteen LEDs** in the same spatial animation; the two directly visible top LEDs receive their own restrained brightness profile and transient-layer emphasis.
- Avoid visible flicker, temporal-dithering shimmer, abrupt steps, hotspots where possible, and conspicuous repetition.
- On power-up, make illumination **arrive from outside the keyboard** as part of the wave animation, rather than switching on instantly or applying a uniform fade.
- Remain modular enough to compare effects without rewriting geometry, colour handling, or split synchronisation.

The intended final enclosure is frosted/translucent. **Physical optical tuning is deferred until after v1.0**, because no representative case is available yet. Preserve tunable parameters and a clean extension point for it, but do not guess the diffuser’s behaviour or block v1.0 on it.

## 2. Hardware and physical geometry

### 2.1 LEDs and functional roles

There are **seven addressable WS2812B-family LEDs per half**: six underglow emitters plus one directly visible top LED. **All seven participate in the shared spatial animation.** The top LED additionally indicates momentary layers using a brighter (but still capped) envelope and a transient colour. The nominal QMK logical layout is:

| Physical half | Underglow indices | Top-indicator index |
| --- | --- | --- |
| Left | 0–5 | 6 |
| Right | 7–12 | 13 |

The two halves use distinct left/right QMK hardware targets. Handedness is assigned per physical half so either may be the USB primary. This specification assumes each board renders its own seven LEDs from a common spatial/time model.

### 2.2 Source geometry

Source: `cantor-pro-mx-pcb.kicad_pcb`, KiCad 8 board file, Cantor Pro MX revision v3. In the PCB file, `LED_Ln` and `LED_Rn` have coincident footprint centres on opposite faces of the reversible PCB. The following are **actual footprint-centre coordinates**, not estimated positions:

| LED number on each half | PCB X (mm) | PCB Y (mm) | Normalised local x | Normalised local y |
| --- | ---: | ---: | ---: | ---: |
| 1 | 163.984 | 69.301 | 0.7569 | 0.1972 |
| 2 | 125.884 | 62.301 | 0.4868 | 0.1476 |
| 3 | 87.784 | 79.301 | 0.2167 | 0.2681 |
| 4 | 87.784 | 98.351 | 0.2167 | 0.4032 |
| 5 | 125.884 | 107.450 | 0.4868 | 0.4677 |
| 6 | 165.246 | 111.331 | 0.7658 | 0.4952 |
| 7 (top) | 183.650 | 107.600 | 0.8963 | 0.4687 |

The board's `Edge.Cuts` geometry has an approximate X extent of **57.209–198.278 mm** (width **141.069 mm**). The normalisation uses this full PCB width as one unit, for **both x and y**, preserving spatial aspect ratio:

- `u = (PCB_x − 57.208721) / 141.069209`
- `v = (PCB_y − 41.476401) / 141.069209`

Schematic from outer edge toward the central gap (physical LED numbers):

```text
LEFT half                         GAP                         RIGHT half
  3 —— 2 —— 1                                              1 —— 2 —— 3
  4 —— 5 —— 6                                              6 —— 5 —— 4
outer       inner                                          inner       outer
```

This schematic expresses ordering, not equal physical spacing: the PCB LED rows are staggered.

### 2.3 Two-half coordinate system and configurable gap

Define **`half_gap`** as the horizontal empty distance between the *PCB bounding boxes*, in units of one half's PCB width. The initial value is **`half_gap = 1.0`**.

For a local LED centre `(u, v)`:

- Physical left global coordinates: `x = u`, `y = v`.
- Physical right global coordinates: `x = 2 + half_gap − u`, `y = v`.

Thus with `half_gap = 1.0`, the left half occupies `x ∈ [0,1]`, the empty gap occupies `[1,2]`, and the right half occupies `[2,3]`. Mirroring the right half places LEDs 1 and 6 near the split on both sides.

**Important:** The effect evaluates a *continuous* field over this entire coordinate system. Nothing special occurs at the gap boundary; the field traverses the unlit space at its normal evolving speed. The nearest opposing underglow LEDs are roughly **1.47 half-width units** apart at the initial gap.

The horizontal gap is configurable without changing LED coordinates or effect algorithms. Vertical offset or rotation of independently positioned halves is out of scope for v1, but geometry should not preclude extending to it.

## 3. Visual layers and palettes

### 3.1 Persistent underglow identity

The six underglow LEDs on each half display colours associated with the persistent typing mode, **irrespective of momentarily active symbol/navigation layers**.

| Persistent mode | Left underglow | Right underglow | Palette source |
| --- | --- | --- | --- |
| `L_BASE` — coder | `waveAqua2` `#7AA89F` | `waveAqua2` `#7AA89F` | Kanagawa; deliberately subdued overall illumination |
| `L_EN` — English | green `#169B62` | orange `#FF883E` | Irish flag digital approximation |
| `L_SE` — Swedish | blue `#006AA7` | yellow `#FFCD00` | Swedish flag digital approximation |
| `L_FR` — French | blue `#000091` | red `#E1000F` | French flag / state-palette approximation |

The coder layer uses a **lower brightness profile**, not a different, greyed-out or HSV-desaturated palette. Existing palette constants are starting references; visible output will depend on LED and diffuser behaviour.

The currently defined `L_TH` and other layers with no agreed persistent palette are not assigned new colours in v1. Their fallback behaviour must be chosen explicitly before implementation.

### 3.2 Animated top LEDs and transient emphasis

**The top LED on each half samples exactly the same spatial animation as the underglow**, at its own physical coordinates (LED 7 in the KiCad geometry). It is not a separate static lamp or a different effect.

| State | Top LED colour | Top LED brightness / motion |
| --- | --- | --- |
| No transient layer | Matches that half's persistent coder/flag palette | Very dim, but **not off** during normal operation; follows the spatial wave |
| `L_NUMSYM` | Kanagawa `oniViolet` `#957FB8` | Raised, configurable brightness envelope; **continues** following the same wave |
| `L_NAV` | Kanagawa `dragonBlue` `#658594` | Raised, configurable brightness envelope; **continues** following the same wave |

Entering a transient layer should feel like the top LEDs coming forward out of the ambient illumination, not as a hard switch. **Fade the brightness increase over approximately 200 ms** (tunable); fade back on release, also without interrupting the animation. The transient colour also transitions smoothly, preferably on a similarly responsive schedule; it must not flash white or snap between hues. The fade changes the top LED's *brightness profile*, not its wave phase or spatial sample. Even at peak emphasis, the top LEDs should remain comfortably dim for direct viewing.

The top LEDs use their **own minimum/maximum brightness and safety cap** because of their direct line of sight. The normal ambient top profile is much dimmer than the underglow. During the power-up entrance sequence (Section 7), top LEDs are initially dark along with the underglow: the nonzero ambient baseline applies **after** the arrival animation has established normal operation.

`L_FRSYM`, `L_FN`, and precedence when several transients overlap remain open decisions.

## 4. Colour and brightness processing

### 4.1 Colour transitions and working space

- Adopt **OKLab as the working colour space** for colour state and interpolation. Do not use HSV for colour management.
- Keep published palette references in canonical **sRGB hex** for traceability (national flags and Kanagawa), and convert these endpoints into OKLab at initialisation or build time. The live morph operates in OKLab.
- Begin with a smooth, eased transition of approximately **1.5–2 seconds**; duration is configurable.
- Transition each half's persistent colour independently but with the **same timing**, so asymmetric flags morph coherently. The ambient top colour follows its own half; transient-top colour changes use the same OKLab interpolation framework but a faster, separately configurable duration.
- If another persistent layer is selected mid-transition, start the next transition from the **colour currently being displayed**, without snapping to either endpoint.
- Apply a defined sRGB gamut-handling policy before output. Avoid unnecessary colour shifts and unintended routes via white/grey.
- **Never reset or pause the animation** merely because the target colour changes.

The pipeline is conceptually **canonical sRGB palette → OKLab working colours → eased colour morph → perceptually designed lightness/brightness envelope → gamut-managed RGB output**. The precise lightness-to-electrical-intensity mapping will be validated in software and on hardware; naïvely multiplying OKLab lightness or encoded sRGB channels is not assumed correct. OKLab does not guarantee that WS2812 output will match reference colours exactly. Exact colour calibration is an optional later refinement.

### 4.2 Brightness

The effect produces a normalised brightness field, **sampled at all fourteen LED centres**, which a separate output stage maps to the LEDs. Persistent modes have independently configurable **minimum** and **maximum** underglow brightness; the minimum must be nonzero and visibly present during **established normal operation**. The two top LEDs have separate ambient and transient-emphasis ranges. **Power-up entrance is a deliberate exception** to the nonzero normal baseline: illumination begins dark and arrives spatially from beyond the sea edge.

Required tunables include:

| Parameter | Meaning |
| --- | --- |
| `underglow_min_language` / `underglow_max_language` | Illuminance envelope for language palettes |
| `underglow_min_coder` / `underglow_max_coder` | Lower-output envelope for coder/base |
| `wave_strength` | Relative contrast of the travelling swell |
| `breath_strength` | Strength of slow global breathing |
| `top_ambient_min` / `top_ambient_max` | Very dim top-LED envelope with no transient layer |
| `top_active_min` / `top_active_max` | Brighter top-LED envelope while a transient layer is active |
| `top_output_cap` | Safety/comfort ceiling for direct-view top LEDs |
| `top_emphasis_fade_duration` | Fade between ambient and active top profiles; initially **200 ms** |
| `top_colour_transition_duration` | Responsive OKLab top-indicator colour morph |
| `colour_transition_duration` | Persistent-palette morph duration |
| `startup_enabled` / `startup_profile` | Select the spatial entrance choreography at power-up |
| `startup_scale` / `startup_duration` | Overall initial wave amplitude/timing adjustment; specifics to tune in prototype |

All values and their units/ranges must be documented and adjustable in one configuration area. The precise electrical-to-perceptual brightness mapping remains a calibration decision. A common scalar applied indiscriminately to sRGB bytes is not automatically perceptually uniform: the pipeline should respect colour-space encoding and avoid accidental double gamma correction.

**No temporal dithering in v1.** Earlier tests produced visible trembling. At low brightness, 8-bit WS2812 quantisation may remain visible; mitigate first through curve shape, an adequate baseline, and update cadence; assess optical diffusion only after a physical case exists.

## 5. Animation framework

### 5.1 Shared effect interface

An effect shall be conceptually a function of **physical position** and **continuous animation time/state**, returning a normalised brightness value. It must not own the current language colour or top-indicator behaviour.

The architecture shall separate:

1. **Geometry** — LED positions, side mirroring, configurable gap.
2. **Time/state** — one logically shared evolving phase and smooth speed variation.
3. **Effect** — Shore initially; Tide and Living Light interchangeably later.
4. **Palette manager** — persistent-layer selection, interruptible OKLab colour morph.
5. **Renderer** — brightness shaping and LED output for underglow and animated top LEDs, with a top-only transient emphasis envelope.
6. **Lifecycle choreography** — power-up arrival and normal operation, with a small, event-driven extension point reserved for a possible future departure effect. No departure sequence or host integration is implemented in v1.0.

Changing the selected effect should require replacing its brightness-field calculation, not its colour or split-handling code. All fourteen LEDs, including the top two, use this same effect. Crossfading between live effects is desirable later, but not a v1 requirement.

### 5.2 Shore — the first effect

A continuous, organic **sea-to-beach swell**:

- `sea_side` is configurable: **left or right**. The opposite side is the beach.
- A **broad, soft illumination swell** travels from sea toward beach across both halves and the actual configurable gap.
- It gradually decelerates/spreads near the beach, then retreats toward its sea origin.
- **Advance and retreat need not be mirror images**: retreat can be flatter and initially quicker, then settle gradually.
- Speed changes smoothly; avoid per-frame randomness, abrupt reversals, fixed stepping, and a conspicuous mechanically repeated cycle.
- Maintain a clearly visible nonzero illumination baseline everywhere, including when the brightest part of a swell is in the unlit gap.
- Add a very restrained **global breathing** component, evolving independently of the travelling swell.
- A subtly oblique wavefront may exploit the staggered 3×2 layout, but start with a near-vertical front.

Initial tuning targets, **not fixed requirements**: approximately **30–50 seconds** for a complete advance-and-retreat cycle; slower long-term modulation of cycle speed; broad wavefront influencing multiple LEDs at once. Wave width, softness, asymmetry, and breathing strength must be tunable.

At a reversal, the wave's position and velocity should evolve continuously (no visual jump). Avoid an instantaneous teleport from beach to sea at cycle boundaries.

### 5.3 Tide and Living Light — future effects

- **Tide:** a broad brightness swell drifting back and forth across the continuous two-half geometry, with smoothly changing speed; less physically directional than Shore.
- **Living Light:** slow asymmetric global breathing plus even slower spatial drift; two or more gentle rhythms that do not produce an obvious short loop.

Both use the same geometry, clock, colour manager and renderer. Shore comes first so effect abstractions can be validated without implementing three animations at once.


### 5.4 Compute and refresh budget (v1.0)

**Normal-operation output is capped at 30 fps** (approximately one new frame every 33⅓ ms). Thirty frames per second is the initial/default target, not a reason to perform unnecessary calculations or LED writes when the visible state is unchanged. Any adjustable refresh setting shall not exceed 30 fps in v1.0. Motion and colour transitions are computed from elapsed shared time, **not** from an assumed fixed number of frames, so modest scheduling jitter or a skipped frame cannot change animation speed or restart the wave.

- Use a **continuous analytical light field**, not a numerical fluid simulation or a maintained grid. Calculate one field sample at each of the **seven physical LED positions per half**, including the top indicator. A broad swell is more important than fine-scale physical realism at this spatial resolution.
- Precompute and store LED geometry in a compact static representation. Prefer fixed-point arithmetic and lookup tables for recurring wave-profile/easing calculations where they improve predictability; do not mandate them if a simpler measured implementation is already inexpensive.
- Keep canonical sRGB/OKLab palette endpoints cached. Compute the *current* palette colour **once per half per frame at most**, reusing it for the local LEDs; avoid seven identical OKLab conversions each frame. Only recompute colour-transition work when it is needed.
- Prepare the complete local **seven-LED output buffer**, then perform **one WS2812 transfer per half per rendered frame**, if supported by this QMK fork. Avoid `rgblight_sethsv_at()` in a seven-call loop when each call independently refreshes the strip. The output transaction should use the RP2040's appropriate hardware-timed driver (e.g. PIO when supported), rather than CPU-timed bit-banging.
- No frame-time memory allocation, per-frame random speed jumps, optical convolution, or per-pixel fluid computation. Keep all work off the keyboard's critical key-scanning and split-communication path as far as practical.
- **Performance goal, to measure rather than assume:** target less than 2% of one RP2040 core's CPU time for lighting computation. Verify actual execution time and key latency on both hardware targets; meeting the 30 fps cap and preserving keyboard responsiveness are the hard acceptance requirements.
- Suspend or a proven low-power state may update **much more slowly or not at all**, independently of the normal 30 fps ceiling. Do not stream full animation frames over the split link when both halves can render from synchronised time and shared parameters.

## 6. Optical model and enclosure — after v1.0

Each LED illuminates an area; under the eventual frosted case, neighbouring illumination zones may overlap. The target is a **continuous luminous surface**, not separate stepping points. **There is no enclosure prototype yet, so optical optimisation is explicitly postponed until after v1.0.**

For v1.0, **sample the smooth mathematical brightness field at each of the fourteen LED centres**. Make the spatial swell broad relative to LED spacing; avoid scattering simulation, ray tracing, and numerical blur in the initial implementation.

To avoid later refactoring, keep the physically meaningful controls in a **single configurable geometry/optics section**, including `half_gap`, wave width, wavefront softness, and the brightness envelopes. Reserve a clearly bounded renderer/field-sampling extension point for future optical compensation. A potential `optical_influence_radius` or area-averaging control may be added once measurements justify it; do not create fictitious calibrated settings or inactive knobs that imply an optical model already exists.

After v1.0, test real material translucency, wall thickness, distance from LEDs to diffuser, internal reflections, and desk-reflected glow. Tune configuration first; add a software spatial filter only if a physical prototype demonstrates a need.

## 7. Split synchronisation, startup and lifecycle

### 7.1 Shared timeline and runtime behaviour

- Both RP2040 halves must sample the **same logical animation timeline** and global spatial field, regardless of which half is USB primary.
- Use QMK's split synchronised-timer facilities **if verified to work in this particular Vial/QMK fork**, otherwise provide a small, explicit phase/time synchronisation mechanism. Do not assume independent free-running clocks remain aligned.
- Layer/palette state must reach both halves; the previous working keyboard required local rendering based on synchronised layer state, rather than assuming the layer-change callback ran on the secondary half.
- Minor time corrections must not produce visible phase jumps. Consider gentle correction or monotonic/slewed time if required.
- The animation must continue uninterrupted on **normal layer changes**, including while colour morphing and while momentary NAV/Symbol layers are held. Top transient emphasis must also leave time and phase untouched.
- A controller reboot or loss of power may reinitialise the shared timeline; preserving phase across a physical power outage is **not** a v1 requirement. Once reconnected, the halves must resynchronise. A secondary half joining late should follow the primary's current entrance/animation state, not independently replay a startup sequence.
- Normal refresh is capped at **30 fps**, with state advanced by shared elapsed time rather than frame number. Batch all seven local LED values into **one output transfer per local frame** where supported by the chosen QMK lighting API (Section 5.4).

### 7.2 Power-up: light arriving from elsewhere

**Do not instantly enable the normal baseline, and do not substitute a simple uniform fade-in.** The intended illusion is that an already-existing ocean of light moves in from beyond the keyboard, finds the keys, and gradually makes the keyboard part of its motion. The animation is not *born inside the LEDs*.

For the initial Shore startup choreography, with the sea direction set by `sea_side`:

1. **Dark, off-stage origin.** At first, all fourteen LEDs are unlit. The field begins beyond the outer sea-side edge, so its first visible effect is spatially localised near that edge rather than simultaneously appearing everywhere.
2. **Tentative first arrival.** A small, soft swell washes onto the sea-side portion of the keyboard. Its reach and peak lightness are deliberately limited; it does not establish the full normal baseline on every LED.
3. **Retreat.** The small swell recedes back toward the sea without abruptly disappearing or reversing velocity.
4. **Confident return.** A larger swell returns, reaches farther across the split gap and the beach side, and gently establishes the configurable persistent baseline as it advances.
5. **Seamless handover.** The field merges into the normal continuous Shore rhythm with no phase teleport, amplitude jump, or restart. The animation clock and colour palette remain shared throughout.

**Startup and ongoing Shore are two phases of one coherent field**, not an unrelated logo animation followed by the actual effect. The transition to the normal nonzero baseline should itself move through space; it must not suddenly illuminate distant LEDs while a wave is in the gap. The top LEDs participate in the same arrival, initially at their much dimmer ambient level, unless a transient layer is already held.

Arrival depth, first-swell scale, pause/retreat character (if any), second-swell amplitude, and total entrance timing are **configurable tuning values**; no durations are frozen before viewing a prototype. At initial power-up, start dark even though normal-operation minima are nonzero. A late secondary half joins the shared stage of the choreography and does not start a separate sea wave.

The entrance is intended when a **cold, powered keyboard first sees an active, configured USB host**, not merely when 5 V arrives at its USB port. Some sleeping laptops keep USB power available: in that situation, the keyboard should remain dark until an active host connection is established, rather than automatically running Shore on power-up. The primary half's host connection state must govern both halves. QMK/USB-state detection in the installed firmware must be verified; host configuration is a useful signal, but not proof that the operating system is awake in every laptop power state.

The entrance should not replay merely because an already-running host re-enumerates the keyboard, nor because a language/transient layer changes. After a genuine USB suspend/resume, replaying the entrance versus resuming a continuous field is an explicit policy to validate on hardware. **Do not conflate electrical power, USB configuration, USB suspend and host sleep.**

### 7.3 Suspend, sleep and unplugging — v1.0 scope

**No animated shutdown or departure in v1.0.** USB suspend is not reliable advance notice of impending power loss, and a physical unplug supplies none. Do not build a Linux service, Raw HID shutdown protocol, delayed sleep command, energy-storage workaround, or shutdown state machine for this version.

- **Powered but not USB-configured:** keep the LEDs off; a sleeping laptop can supply 5 V without providing an active host connection. Do not trigger Shore merely on power-up.
- **Active, configured USB host:** permit the cold-start Shore entrance (if not already completed); normal operation runs at no more than 30 fps.
- **Actual USB bus suspend:** promptly turn the LEDs off and comply with the applicable low-power/current limit. Do not assume there is time for a shutdown animation.
- **Optional, conditional sleep-breathing concept:** if a particular host retains power *and an operating mode permitting the consumption* while it sleeps, we may use very soft, slow breathing in that mode, at a much lower update rate. Its feasibility requires measuring current consumption and observing USB states on the assembled keyboard. Seeing LEDs lit during laptop sleep establishes available power, **not** USB-suspend current compliance. This effect is **not required for v1.0** and must not take precedence over a genuine suspended-bus low-power requirement.
- On cable removal or sudden power loss, immediate darkness is expected.
- Keep cold active-host arrival distinct from USB re-enumeration and resume; investigate actual resume behaviour to avoid unwanted relaunches.
- **Future-proofing requirement:** keep the startup/normal effect lifecycle separate from palette changes and from QMK event detection. A later caller should be able to request a `departure` transition from the *current* spatial field without refactoring geometry, colour interpolation, or split timing. This is an architectural seam, **not** a requirement to implement a generic lifecycle state machine or unused hooks now.

The optional keyboard-initiated departure scenario is recorded in **Section 12, Someday**; it is not part of v1.0 acceptance.

## 8. Acceptance criteria

| Test | Expected result |
| --- | --- |
| Coder ⇄ English ⇄ Swedish ⇄ French | Palette morphs smoothly over the selected interval, from the actually displayed current colour |
| Change language during a swell or during an existing morph | Motion/phase is not restarted, paused, or jumped |
| Normal typing with no transient | Both top LEDs faintly follow their own half's palette and the **same spatial wave**, rather than being off |
| Hold/release `L_NUMSYM` | Both top LEDs morph toward violet and fade to/from their brighter profile over about **200 ms**; their wave motion and underglow continue |
| Hold/release `L_NAV` | Both top LEDs morph toward dragonBlue and fade to/from their brighter profile over about **200 ms**; their wave motion and underglow continue |
| Top-LED wave sampling | Top LEDs participate at their actual LED-7 coordinates; no separate phase or independent pulse |
| Power up a cold keyboard with an active USB host | Initially dark; a small wave arrives from the sea side, recedes, then a larger wave establishes the normal baseline spatially; no full-keyboard instant-on |
| Connect to a sleeping laptop that supplies USB power but does not configure an active USB device | LEDs remain off; Shore does not start simply because 5 V is present |
| Second half joins during entrance | It displays the correct shared time and stage rather than independently starting its own entrance |
| USB host re-enumerates a still-running keyboard | No gratuitous replay of startup animation or phase restart |
| Physical unplug / sudden power loss | Immediate darkness is expected; no shutdown animation |
| USB suspend | LEDs comply promptly with the applicable low-power behaviour; no aesthetic departure is required |
| Optional host-asleep breathing (only if implemented later) | Very dim, slow, and limited to verified permissible power conditions; real USB suspend still takes priority |
| Wave crosses keyboard centre | The empty gap adds the configured travel time; no instantaneous transfer of the peak |
| `half_gap` is changed | Travel timing changes without editing the LED coordinate table |
| Swap `sea_side` | Wave advances from the newly selected sea side and retreats toward it |
| Advance reaches beach / retreat ends | No position discontinuity, sudden velocity reversal, or hard loop boundary |
| Coder mode at low brightness | Baseline remains visible; no flicker, trembling, or objectionable quantised stepping |
| Bright language mode | Swell is perceptible but soft; no rapid, distracting pulsing |
| Either half used as USB primary | Physical left/right colours and animation geometry remain correct; halves remain coherent |
| Future diffuser prototype (**post-v1.0**) | Prefer blended ambient glow over distinct points; tune accessible parameters before considering software blur |
| Alternate effect selected in source | Geometry, synchronisation, palettes, and top LEDs need no rewrite |
| Normal rendering performance | Output never exceeds **30 fps**; one batched seven-LED output per half per frame; measured computation/transport does not degrade typing or split reliability |
| Reduced frame rate / scheduler jitter | Wave progression and OKLab morphing retain their correct elapsed-time behaviour instead of slowing or speeding with frame count |

Visual acceptance should be assessed in **normal daytime and dim-room lighting**, first without and then with a representative diffuser. A desk video with fixed exposure can be used for comparison, but subjective viewing is the decisive test.

## 9. Non-goals for v1

- Per-LED random sparkle, rainbow cycling, rapid reactive effects, or audio response.
- Frame-by-frame pseudorandom speed changes or temporal brightness dithering.
- Optical ray tracing, LED emission simulation, or automatic calibration from a camera; enclosure-driven optical tuning is postponed until **after v1.0**.
- Runtime graphical animation editors or complex on-keyboard controls.
- Perfect colourimetric matching of physical LEDs to national-flag standards.
- Guaranteed animation-phase preservation across loss of power or MCU reset; any graceful departure or shutdown animation (see Someday).
- Simultaneous implementation of all planned effects.

## 10. Open decisions before coding

1. **Time synchronisation:** validate the available split clock and transport in the installed QMK/Vial fork; measure drift and correction behaviour.
2. **Brightness semantics:** choose consistent parameter units and an sRGB/linear-light/perceptual-lightness output pipeline, then tune coder versus language minima and maxima.
3. **Shore envelope:** settle wave width, softness, advance/retreat asymmetry, speed modulation, global breathing, and starting sea side using a first prototype.
4. **Top-LED behaviour:** tune ambient versus active brightness envelopes, direct-view safety cap, roughly 200 ms emphasis fades, top-colour morph duration, and interaction with the shared wave.
5. **Startup Shore choreography:** tune small arrival, retreat, expanded return and spatial establishment of the normal baseline; ensure a smooth handover to ongoing Shore.
6. **Additional layers:** decide treatment of `L_TH`, `L_FRSYM`, `L_FN`, and transient-indicator precedence.
7. **LED output API and measured cost:** confirm how to batch seven WS2812 values into one transfer per half at **≤30 fps** with this `rgblight` driver; verify CPU cost and that typing/split latency is unaffected.
8. **Colour gamut:** validate the OKLab-to-RGB lightness, gamut and 8-bit output pipeline; test key pairs such as Swedish yellow → French red.
9. **Optics (post-v1.0):** test real frosted material, thickness, LED-to-wall separation, and desk-reflected glow; keep geometry, wave width, softness and envelopes configurable in v1.0.
10. **USB host-state and suspend/resume safety:** verify how to distinguish powered-but-unconfigured, configured-and-active, actual USB suspend, wake, re-enumeration and cable removal (including any effects of `NO_USB_STARTUP_CHECK`); ensure safe low-power operation. A host-sleep breathing effect is optional and only permitted after current/power validation. Graceful departure remains out of scope.

## 11. Recommended order of validation (not an implementation prescription)

1. Verify physical LED addressing and global coordinates against the actual assembled keyboard.
2. Verify both halves track one continuously evolving shared timebase.
3. Render a single test spatial field on all fourteen LEDs at **no more than 30 fps**, batched into one output transfer per half; verify top-LED position, output caps, CPU time and keyboard responsiveness.
4. Introduce ordinary Shore motion and tune wave timing/softness, nonzero normal baseline, and top emphasis fades without changing palette.
5. Add independently interruptible OKLab palette morphing, keeping wave phase intact for all fourteen LEDs.
6. Add the **cold-start entrance choreography** (tentative arrival, retreat, larger return, spatial baseline establishment) and verify smooth handover to ordinary Shore.
7. Validate powered-but-unconfigured USB versus active-host cold start, re-enumeration and safe suspend/resume behaviour. Do not build a shutdown animation, host integration or unvalidated suspend breathing.
8. Freeze v1.0 for use without an enclosure. **After v1.0**, evaluate physical diffusion and tune the reserved optical configuration/extension point.
9. Swap in Tide or Living Light using the same shared infrastructure.

## 12. Someday — keyboard-initiated graceful departure

**Deferred feature, not a v1.0 requirement.** If the user's keymap ever contains a firmware-handled sleep/suspend/hibernate-like key action (for example, `KC_SYSTEM_SLEEP`), QMK could intercept that *keyboard-initiated* action before forwarding the request to the host:

1. Snapshot the current animation position, wave velocity, colour morph and brightness state.
2. Steer the existing Shore field gently into a sea-ward withdrawal, tapering its baseline as the light leaves the keyboard; do not restart a separate animation.
3. After a configurable, short departure interval, send the original sleep request to the host. The host may handle or ignore that request; this is not a general solution for host-initiated sleep.

This future feature would require explicit user enablement and testing of the actual QMK keycode and OS behaviour. A sleep key is **a possible trigger**, not a guaranteed notification that the host will sleep, and should not be delayed unless that behaviour is wanted. No host-specific daemon, automatic shutdown interception, or power-loss hardware is planned.

**Keep the door open without feature creep:** effect/state calculation should not be hardwired to just the startup path, but v1.0 should implement only **startup arrival → continuous normal Shore** and safe power-state handling. Defer the `departure` transition and any key interception until there's a concrete reason to build them.

---

**Review checkpoint (v0.4):** Shore rendering is capped at **30 fps**, using an analytical field, cached palette work and one batched local LED transfer per frame. Cold USB power without an active host must not trigger the entrance. Very dim sleep breathing remains an optional, power-validated future extension; actual bus suspend requires safe low-power handling. Top LEDs still share the field, OKLab remains the colour space, and graceful departure remains Someday. No firmware is changed by this document.
