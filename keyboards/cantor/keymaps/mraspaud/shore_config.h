// Copyright 2026 Martin Raspaud
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// All spatial coordinates are in units of one PCB half-width.
#define SHORE_HALF_GAP                 1.0f
#define SHORE_SEA_ON_LEFT             1     // 0 = sea on physical right
#define SHORE_WAVE_HALF_WIDTH         0.85f // broad enough for the 3x2 LEDs
#define SHORE_WAVE_TILT               0.065f
#define SHORE_WAVE_STRENGTH           0.95f
#define SHORE_BREATH_STRENGTH         0.05f

// All brightness values are *linear-light multipliers* in [0,1], NOT HSV V.
// As in the spec, separate ambient envelopes for coder and languages.
// Underglow minima are spatially varying: sea side stays more alive,
// beach side dries out after the wave has receded.
#define SHORE_LANGUAGE_MIN_SEA        0.07f
#define SHORE_LANGUAGE_MIN_BEACH      0.015f
#define SHORE_LANGUAGE_MAX            0.72f

#define SHORE_CODER_MIN_SEA           0.040f
#define SHORE_CODER_MIN_BEACH         0.010f
#define SHORE_CODER_MAX               0.20f

// Reduce idle ambient motion at the beach so that the wave can truly recede.
// 1.0 = same ambient everywhere; smaller values = drier beach.
#define SHORE_BEACH_AMBIENT_SCALE     0.22f


// How far the swell centre overshoots beyond the illuminated field.
// Smaller than before, so the wave stays visually engaged with the LEDs.
#define SHORE_WAVE_OVERSHOOT          0.18f

// Wavefront accelerates/decelerates briefly, with mostly steady travel.
#define SHORE_V2_EDGE_RAMP             0.12f

// Shore v2: coherent, correlated waves rather than a fixed repeating cycle.
// Strength, breadth, travel duration and settling are derived from wave size.
// Sea state changes slowly, producing occasional groups of larger swells.
#define SHORE_V2_FORMATION_MIN_MS      1200u
#define SHORE_V2_FORMATION_MAX_MS      2400u
#define SHORE_V2_TRAVEL_SLOW_MS       23000u // smaller waves travel more slowly
#define SHORE_V2_TRAVEL_FAST_MS       15000u // larger waves travel faster
#define SHORE_V2_TRAVEL_JITTER_MS      1600u // +/- half of this range
#define SHORE_V2_SETTLE_SMALL_MS       2600u
#define SHORE_V2_SETTLE_LARGE_MS       5400u
#define SHORE_V2_CALM_MIN_MS           2400u
#define SHORE_V2_CALM_MAX_MS           7100u
#define SHORE_V2_SEA_STATE_MEMORY      0.78f // near 1 = longer groups
#define SHORE_V2_SIZE_SLEW             0.24f // maximum change from previous wave
#define SHORE_V2_BREAK_WASH            0.26f // broad wash after the crest breaks
#define SHORE_V2_CALM_BREATH           0.004f // common near-stillness, no spatial ripple
#define SHORE_TOP_AMBIENT_MIN         0.0030f
#define SHORE_TOP_AMBIENT_MAX         0.011f
#define SHORE_TOP_ACTIVE_MIN          0.024f
#define SHORE_TOP_ACTIVE_MAX          0.075f
#define SHORE_TOP_OUTPUT_CAP          0.13f

#define SHORE_PALETTE_MORPH_MS        1800u
// Top LEDs keep their local palette colour; only brightness responds.
#define SHORE_TOP_FADE_IN_MS          400u
#define SHORE_TOP_FADE_OUT_MS         2000u

// 34 ms gives 29.41 fps, always < 30 fps. Effect time is independent of frames.
#define SHORE_FRAME_MS                34u
#define SHORE_SPLIT_SYNC_MS           250u
#define SHORE_SPLIT_TIMEOUT_MS        1200u

// Entrance: small wave -> retreat -> stronger return -> blend to normal.
#define SHORE_ENTRANCE_FIRST_MS       2600u
#define SHORE_ENTRANCE_RETREAT_MS     4900u
#define SHORE_ENTRANCE_BLEND_MS       12900u
#define SHORE_ENTRANCE_END_MS         15900u

// Startup: the returning wave illuminates a broad, soft region before the
// crest arrives, rather than activating individual LED columns abruptly.
// All lengths are in units of one PCB half-width.
#define SHORE_ENTRANCE_LEAD            0.50f
#define SHORE_ENTRANCE_FEATHER         1.50f
#define SHORE_ENTRANCE_TILT            0.22f
#define SHORE_FIRST_WAVE_OPACITY       1.50f

// Zero means dark whenever the bus isn't configured/active.
#define SHORE_REQUIRE_CONFIGURED_USB  1

#define SHORE_UNDERCURRENT_STRENGTH   0.16f
#define SHORE_UNDERCURRENT_PERIOD_MS 17000u
#define SHORE_UNDERCURRENT_LAG_MS     2800u

// BASE / coder YlGnBu_r: edit these 24-bit sRGB hex colours directly.
// Physical LED numbers 1..7 on each half; 7 is the top indicator.
// SEA and BEACH describe roles; SHORE_SEA_ON_LEFT swaps them.
// LEDs 1 and 6 are closest to the split gap.
#define SHORE_BASE_SEA_LED_1          0x1F92C0u
#define SHORE_BASE_SEA_LED_2          0x225CA7u
#define SHORE_BASE_SEA_LED_3          0x21318Cu
#define SHORE_BASE_SEA_LED_4          0x21318Cu
#define SHORE_BASE_SEA_LED_5          0x225CA7u
#define SHORE_BASE_SEA_LED_6          0x2094C0u
#define SHORE_BASE_SEA_LED_7          0x33A7C3u

#define SHORE_BASE_BEACH_LED_1        0x7FCDBBu
#define SHORE_BASE_BEACH_LED_2        0xCAEAB4u
#define SHORE_BASE_BEACH_LED_3        0xF0F9B7u
#define SHORE_BASE_BEACH_LED_4        0xF0F9B7u
#define SHORE_BASE_BEACH_LED_5        0xCAEAB4u
#define SHORE_BASE_BEACH_LED_6        0x7DCCBCu
#define SHORE_BASE_BEACH_LED_7        0x5FC0C1u

// Flag colour chroma: 1.0 = original, 0.0 = greyscale
#define SHORE_FLAG_CHROMA 0.90f

// Organic colour movement, linked to the undercurrent.
#define SHORE_CHROMA_MOTION   0.025f  // +/- 2.5%
#define SHORE_HUE_MOTION_DEG  0.5f    // +/- 0.5 degrees
