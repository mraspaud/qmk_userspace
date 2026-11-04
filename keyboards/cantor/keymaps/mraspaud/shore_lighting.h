// Copyright 2026 Martin Raspaud
// SPDX-License-Identifier: GPL-2.0-or-later
// Shore ambient lighting for the Cantor Pro MX (RP2040). Include ONCE in keymap.c
// *after* the L_BASE, L_EN, L_SE, L_FR, L_NUMSYM and L_NAV layer enum.
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "shore_config.h"
#include "rgblight.h"
#include "rgblight_drivers.h"
#include "sync_timer.h"
#include "transactions.h"
#include "split_util.h"
#include "timer.h"
#include "usb_device_state.h"

// The QMK RGBLIGHT driver uses LOCAL physical WS2812 indices (0..6 on
// either half) here; global RGBLIGHT indices 7..13 are used by rgblight
// itself, but bypassed deliberately to batch ONE transfer per frame.
#define SHORE_LOCAL_LEDS 7

typedef struct { float l, a, b; } shore_lab_t;
typedef struct { uint32_t start; shore_lab_t from, to; } shore_morph_t;
typedef struct { uint32_t start; float from, to; } shore_scalar_morph_t;
typedef struct {
    uint32_t active_epoch; // QMK synchronised millisecond clock, set by USB primary
    uint8_t active;        // 1: configured and awake; 0: dark
} shore_packet_t;

static shore_packet_t shore_remote = {0};
static uint32_t shore_remote_received_at = 0;
static uint32_t shore_last_send = 0;
static uint32_t shore_epoch = 0;
static bool shore_master_active = false;
static bool shore_ever_active = false;
static uint32_t shore_last_active_at = 0;
static uint32_t shore_last_frame = 0;
static bool shore_last_output_dark = false;
static bool shore_inited = false;
static int8_t shore_language_id = -1;
static int8_t shore_top_id = -1;
static shore_morph_t shore_persistent_morph;
static shore_morph_t shore_centre_morph; // white on LED 1/6/7 in FR and EN
static shore_scalar_morph_t shore_top_emphasis;
static uint32_t shore_top_fade_duration_ms = SHORE_TOP_FADE_IN_MS;
static shore_scalar_morph_t shore_language_brightness;

static inline float shore_clamp(float x, float lo, float hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}
static inline float shore_smooth(float x) {
    x = shore_clamp(x, 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}
static shore_lab_t shore_lerp_lab(shore_lab_t a, shore_lab_t b, float f) {
    return (shore_lab_t){a.l + (b.l-a.l)*f, a.a+(b.a-a.a)*f, a.b+(b.b-a.b)*f};
}

// Canonical sRGB references, transformed to OKLab offline (D65).
// This avoids cubic roots and sRGB decoding on the RP2040.
// Barely warm white, #FFFEFC
static const shore_lab_t SHORE_WHITE = {
    0.997231040f, 0.000269973f, 0.002834227f
};
static const shore_lab_t SHORE_CODE = {0.695899430f, -0.051122905f, -0.001163832f}; // #7AA89F
static const shore_lab_t SHORE_IE_GREEN = {0.609518492f, -0.127433433f, 0.051848732f}; // #169B62
static const shore_lab_t SHORE_IE_ORANGE = {0.746929146f, 0.109309060f, 0.127196889f}; // #FF883E
static const shore_lab_t SHORE_SE_BLUE = {0.505782895f, -0.054277928f, -0.113597406f}; // #006AA7
static const shore_lab_t SHORE_SE_YELLOW = {0.867152251f, -0.002621580f, 0.177236818f}; // #FFCD00
static const shore_lab_t SHORE_FR_BLUE = {0.296817722f, -0.021313088f, -0.204566966f}; // #000091
static const shore_lab_t SHORE_FR_RED = {0.571754331f, 0.206225118f, 0.110077406f}; // #E1000F

// sin(2*pi*n/256), Q15. Linear interpolation hides table steps.
static const int16_t shore_sin_q15[256] = {
    0, 804, 1608, 2410, 3212, 4011, 4808, 5602, 6393, 7179, 7962, 8739,
    9512, 10278, 11039, 11793, 12539, 13279, 14010, 14732, 15446, 16151, 16846, 17530,
    18204, 18868, 19519, 20159, 20787, 21403, 22005, 22594, 23170, 23731, 24279, 24811,
    25329, 25832, 26319, 26790, 27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956,
    30273, 30571, 30852, 31113, 31356, 31580, 31785, 31971, 32137, 32285, 32412, 32521,
    32609, 32678, 32728, 32757, 32767, 32757, 32728, 32678, 32609, 32521, 32412, 32285,
    32137, 31971, 31785, 31580, 31356, 31113, 30852, 30571, 30273, 29956, 29621, 29268,
    28898, 28510, 28105, 27683, 27245, 26790, 26319, 25832, 25329, 24811, 24279, 23731,
    23170, 22594, 22005, 21403, 20787, 20159, 19519, 18868, 18204, 17530, 16846, 16151,
    15446, 14732, 14010, 13279, 12539, 11793, 11039, 10278, 9512, 8739, 7962, 7179,
    6393, 5602, 4808, 4011, 3212, 2410, 1608, 804, 0, -804, -1608, -2410,
    -3212, -4011, -4808, -5602, -6393, -7179, -7962, -8739, -9512, -10278, -11039, -11793,
    -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530, -18204, -18868, -19519, -20159,
    -20787, -21403, -22005, -22594, -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790,
    -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956, -30273, -30571, -30852, -31113,
    -31356, -31580, -31785, -31971, -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757,
    -32767, -32757, -32728, -32678, -32609, -32521, -32412, -32285, -32137, -31971, -31785, -31580,
    -31356, -31113, -30852, -30571, -30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683,
    -27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731, -23170, -22594, -22005, -21403,
    -20787, -20159, -19519, -18868, -18204, -17530, -16846, -16151, -15446, -14732, -14010, -13279,
    -12539, -11793, -11039, -10278, -9512, -8739, -7962, -7179, -6393, -5602, -4808, -4011,
    -3212, -2410, -1608, -804,
};

// Linear-light to sRGB 8-bit, index = round(linear * 256). No powf.
static const uint8_t shore_gamma_u8[257] = {
    0, 13, 22, 28, 34, 38, 42, 46, 49, 53, 56, 58, 61, 64, 66, 68,
    71, 73, 75, 77, 79, 81, 83, 85, 86, 88, 90, 91, 93, 95, 96, 98,
    99, 101, 102, 103, 105, 106, 107, 109, 110, 111, 113, 114, 115, 116, 118, 119,
    120, 121, 122, 123, 124, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136,
    137, 138, 139, 140, 141, 142, 143, 144, 145, 145, 146, 147, 148, 149, 150, 151,
    152, 153, 153, 154, 155, 156, 157, 158, 158, 159, 160, 161, 162, 162, 163, 164,
    165, 166, 166, 167, 168, 169, 169, 170, 171, 172, 172, 173, 174, 174, 175, 176,
    177, 177, 178, 179, 179, 180, 181, 181, 182, 183, 184, 184, 185, 186, 186, 187,
    188, 188, 189, 189, 190, 191, 191, 192, 193, 193, 194, 195, 195, 196, 196, 197,
    198, 198, 199, 199, 200, 201, 201, 202, 202, 203, 204, 204, 205, 205, 206, 207,
    207, 208, 208, 209, 209, 210, 211, 211, 212, 212, 213, 213, 214, 214, 215, 216,
    216, 217, 217, 218, 218, 219, 219, 220, 220, 221, 221, 222, 223, 223, 224, 224,
    225, 225, 226, 226, 227, 227, 228, 228, 229, 229, 230, 230, 231, 231, 232, 232,
    233, 233, 234, 234, 235, 235, 236, 236, 237, 237, 238, 238, 239, 239, 239, 240,
    240, 241, 241, 242, 242, 243, 243, 244, 244, 245, 245, 246, 246, 246, 247, 247,
    248, 248, 249, 249, 250, 250, 251, 251, 251, 252, 252, 253, 253, 254, 254, 255,
    255,
};

// Continuous oscillator computed from elapsed milliseconds, never from frames.
// Using a 256-value sine table avoids firmware trig calls and their cost.
static float shore_sin_ms(uint32_t ms, uint32_t period) {
    float p = (float)(ms % period) * (256.0f / (float)period);
    uint8_t i = (uint8_t)p;
    float f = p - (float)i;
    return ((float)shore_sin_q15[i] +
            ((float)shore_sin_q15[(uint8_t)(i+1u)] - (float)shore_sin_q15[i]) * f) / 32767.0f;
}

// LINEAR sRGB conversion. The light field is applied to linear light, not
// to gamma-encoded sRGB bytes. Clamp any out-of-gamut intermediate channel.
static uint8_t shore_encode_linear(float v) {
    v = shore_clamp(v, 0.0f, 1.0f);
    float index = v * 256.0f;
    uint16_t i = (uint16_t)index;
    float f = index - (float)i;
    uint8_t lo = shore_gamma_u8[i];
    uint8_t hi = shore_gamma_u8[i < 256 ? i + 1 : 256];
    return (uint8_t)(lo + ((float)hi - (float)lo)*f + 0.5f);
}
typedef struct { float r, g, b; } shore_linear_rgb_t;
static shore_linear_rgb_t shore_linear_from_lab(shore_lab_t lab) {
    float l = lab.l + 0.3963377774f * lab.a + 0.2158037573f * lab.b;
    float m = lab.l - 0.1055613458f * lab.a - 0.0638541728f * lab.b;
    float s = lab.l - 0.0894841775f * lab.a - 1.2914855480f * lab.b;
    l = l*l*l; m = m*m*m; s = s*s*s;
    return (shore_linear_rgb_t){
         4.0767416621f*l - 3.3077115913f*m + 0.2309699292f*s,
        -1.2684380046f*l + 2.6097574011f*m - 0.3413193965f*s,
        -0.0041960863f*l - 0.7034186147f*m + 1.7076147010f*s
    };
}
static void shore_rgb_scaled(shore_linear_rgb_t rgb, float intensity,
                             uint8_t *r, uint8_t *g, uint8_t *b) {
    *r = shore_encode_linear(rgb.r * intensity);
    *g = shore_encode_linear(rgb.g * intensity);
    *b = shore_encode_linear(rgb.b * intensity);
}

static float shore_morph_progress(uint32_t now, uint32_t start, uint32_t duration) {
    uint32_t elapsed = now - start;
    return elapsed >= duration ? 1.0f : shore_smooth((float)elapsed/(float)duration);
}
static shore_lab_t shore_morph_sample(shore_morph_t *m, uint32_t now, uint32_t duration) {
    return shore_lerp_lab(m->from, m->to, shore_morph_progress(now, m->start, duration));
}
static float shore_scalar_sample(shore_scalar_morph_t *m, uint32_t now, uint32_t duration) {
    float f = shore_morph_progress(now, m->start, duration);
    return m->from + (m->to - m->from)*f;
}

static int8_t shore_language(layer_state_t state) {
    if (layer_state_cmp(state, L_SE)) return 2;
    if (layer_state_cmp(state, L_FR)) return 3;
    if (layer_state_cmp(state, L_EN)) return 1;
    return 0; // L_BASE and other persistent modes (e.g. L_TH)
}
static shore_lab_t shore_persistent_target(uint8_t language, bool left) {
    switch (language) {
        case 1: return left ? SHORE_IE_GREEN : SHORE_IE_ORANGE;
        case 2: return left ? SHORE_SE_BLUE : SHORE_SE_YELLOW;
        case 3: return left ? SHORE_FR_BLUE : SHORE_FR_RED;
        default: return SHORE_CODE;
    }
}
// Physical LEDs 1 and 6 (zero-based 0 and 5) are nearest the split.
// LED 7 (zero-based 6) is the direct-view top indicator.
// In French and Irish modes these three form the white centre of the flag.
static shore_lab_t shore_centre_target(uint8_t language, bool left) {
    if (language == 1 || language == 3) return SHORE_WHITE; // Irish/French
    return shore_persistent_target(language, left);
}
static int8_t shore_top_mode(layer_state_t state) {
    if (layer_state_cmp(state, L_NUMSYM) || layer_state_cmp(state, L_FRSYM)) return 1;
    if (layer_state_cmp(state, L_NAV)) return 2;
    return 0;
}

static void shore_palette_init(uint32_t now, layer_state_t state, bool left) {
    shore_language_id = shore_language(state);
    shore_top_id = shore_top_mode(state);
    shore_lab_t base = shore_persistent_target((uint8_t)shore_language_id, left);
    shore_persistent_morph = (shore_morph_t){now, base, base};
    shore_lab_t centre = shore_centre_target((uint8_t)shore_language_id, left);
    shore_centre_morph = (shore_morph_t){now, centre, centre};
    float emphasis = shore_top_id ? 1.0f : 0.0f;
    shore_top_emphasis = (shore_scalar_morph_t){now, emphasis, emphasis};
    shore_top_fade_duration_ms = shore_top_id ? SHORE_TOP_FADE_IN_MS : SHORE_TOP_FADE_OUT_MS;
    float language_level = shore_language_id ? 1.0f : 0.0f;
    shore_language_brightness = (shore_scalar_morph_t){now, language_level, language_level};
}

static void shore_palette_sample(uint32_t now, layer_state_t state, bool left,
                                  shore_lab_t *base, shore_lab_t *centre, shore_lab_t *top, float *emphasis,
                                  float *language_level) {
    int8_t new_language = shore_language(state);
    if (shore_language_id < 0) shore_palette_init(now, state, left);
    if (new_language != shore_language_id) {
        shore_lab_t current = shore_morph_sample(&shore_persistent_morph, now, SHORE_PALETTE_MORPH_MS);
        shore_persistent_morph = (shore_morph_t){now, current, shore_persistent_target((uint8_t)new_language, left)};
        shore_lab_t current_centre = shore_morph_sample(&shore_centre_morph, now, SHORE_PALETTE_MORPH_MS);
        shore_centre_morph = (shore_morph_t){now, current_centre, shore_centre_target((uint8_t)new_language, left)};
        float current_level = shore_scalar_sample(&shore_language_brightness, now, SHORE_PALETTE_MORPH_MS);
        shore_language_brightness = (shore_scalar_morph_t){now, current_level, new_language ? 1.0f : 0.0f};
        shore_language_id = new_language;
    }
    *base = shore_morph_sample(&shore_persistent_morph, now, SHORE_PALETTE_MORPH_MS);
    *centre = shore_morph_sample(&shore_centre_morph, now, SHORE_PALETTE_MORPH_MS);

    int8_t new_top = shore_top_mode(state);
    if (new_top != shore_top_id) {
        // Begin the next fade from the currently displayed brightness,
        // including when a new transient interrupts a previous fade-out.
        float current = shore_scalar_sample(&shore_top_emphasis, now, shore_top_fade_duration_ms);
        shore_top_fade_duration_ms = new_top ? SHORE_TOP_FADE_IN_MS : SHORE_TOP_FADE_OUT_MS;
        shore_top_emphasis = (shore_scalar_morph_t){now, current, new_top ? 1.0f : 0.0f};
        shore_top_id = new_top;
    }
    // No transient colour morph: top always tracks its normal palette.
    *top = *centre;
    *emphasis = shore_scalar_sample(&shore_top_emphasis, now, shore_top_fade_duration_ms);
    *language_level = shore_scalar_sample(&shore_language_brightness, now, SHORE_PALETTE_MORPH_MS);
}

// KiCad PCB footprint centres normalised to a half-width. The seventh is
// the visible top LED. The PCB is reversible: right geometry is mirrored.
static const float shore_u[SHORE_LOCAL_LEDS] = {0.7569f,0.4868f,0.2167f,0.2167f,0.4868f,0.7658f,0.8963f};
static const float shore_v[SHORE_LOCAL_LEDS] = {0.1972f,0.1476f,0.2681f,0.4032f,0.4677f,0.4952f,0.4687f};

static float shore_total_width(void) { return 2.0f + SHORE_HALF_GAP; }
static float shore_from_sea(float u, bool left) {
    float x = left ? u : (2.0f + SHORE_HALF_GAP - u);
    return SHORE_SEA_ON_LEFT ? x : shore_total_width() - x;
}

static float shore_alongshore(float x) {
    return shore_clamp(x / shore_total_width(), 0.0f, 1.0f);
}

// 0 at the sea, 1 at the beach, with a slightly stronger pull near the beach.
static float shore_beachness(float x) {
    float t = shore_smooth(shore_alongshore(x));
    return shore_smooth(t);
}
// BASE palette: explicit colour per physical LED, with an editable sRGB
// reference in shore_config.h. Convert to OKLab just once per controller,
// not on every frame. Only seven colours are cached on each RP2040.
static const uint32_t shore_base_hex[2][SHORE_LOCAL_LEDS] = {
    {SHORE_BASE_SEA_LED_1, SHORE_BASE_SEA_LED_2, SHORE_BASE_SEA_LED_3,
     SHORE_BASE_SEA_LED_4, SHORE_BASE_SEA_LED_5, SHORE_BASE_SEA_LED_6,
     SHORE_BASE_SEA_LED_7},
    {SHORE_BASE_BEACH_LED_1, SHORE_BASE_BEACH_LED_2, SHORE_BASE_BEACH_LED_3,
     SHORE_BASE_BEACH_LED_4, SHORE_BASE_BEACH_LED_5, SHORE_BASE_BEACH_LED_6,
     SHORE_BASE_BEACH_LED_7}
};

// Exact 8-bit sRGB -> linear-light lookup. Fixed table avoids libm powf().
static const uint16_t shore_srgb_linear_q16[256] = {
        0,    20,    40,    60,    80,    99,   119,   139,   159,   179,   199,   219,
      241,   264,   288,   313,   340,   367,   396,   427,   458,   491,   526,   562,
      599,   637,   677,   718,   761,   805,   851,   898,   947,   997,  1048,  1101,
     1156,  1212,  1270,  1330,  1391,  1453,  1517,  1583,  1651,  1720,  1790,  1863,
     1937,  2013,  2090,  2170,  2250,  2333,  2418,  2504,  2592,  2681,  2773,  2866,
     2961,  3058,  3157,  3258,  3360,  3464,  3570,  3678,  3788,  3900,  4014,  4129,
     4247,  4366,  4488,  4611,  4736,  4864,  4993,  5124,  5257,  5392,  5530,  5669,
     5810,  5953,  6099,  6246,  6395,  6547,  6700,  6856,  7014,  7174,  7335,  7500,
     7666,  7834,  8004,  8177,  8352,  8528,  8708,  8889,  9072,  9258,  9445,  9635,
     9828, 10022, 10219, 10417, 10619, 10822, 11028, 11235, 11446, 11658, 11873, 12090,
    12309, 12530, 12754, 12980, 13209, 13440, 13673, 13909, 14146, 14387, 14629, 14874,
    15122, 15371, 15623, 15878, 16135, 16394, 16656, 16920, 17187, 17456, 17727, 18001,
    18277, 18556, 18837, 19121, 19407, 19696, 19987, 20281, 20577, 20876, 21177, 21481,
    21787, 22096, 22407, 22721, 23038, 23357, 23678, 24002, 24329, 24658, 24990, 25325,
    25662, 26001, 26344, 26688, 27036, 27386, 27739, 28094, 28452, 28813, 29176, 29542,
    29911, 30282, 30656, 31033, 31412, 31794, 32179, 32567, 32957, 33350, 33745, 34143,
    34544, 34948, 35355, 35764, 36176, 36591, 37008, 37429, 37852, 38278, 38706, 39138,
    39572, 40009, 40449, 40891, 41337, 41785, 42236, 42690, 43147, 43606, 44069, 44534,
    45002, 45473, 45947, 46423, 46903, 47385, 47871, 48359, 48850, 49344, 49841, 50341,
    50844, 51349, 51858, 52369, 52884, 53401, 53921, 54445, 54971, 55500, 56032, 56567,
    57105, 57646, 58190, 58737, 59287, 59840, 60396, 60955, 61517, 62082, 62650, 63221,
    63795, 64372, 64952, 65535
};

// The cube-root inputs are in [0,1]. Binary search runs only during palette
// initialisation (21 cube roots per half), never in the animation loop.
static float shore_unit_cbrt(float v) {
    float lo = 0.0f, hi = 1.0f;
    for (uint8_t j = 0; j < 17; ++j) {
        float mid = 0.5f * (lo + hi);
        if (mid * mid * mid < v) lo = mid;
        else hi = mid;
    }
    return 0.5f * (lo + hi);
}

static shore_lab_t shore_hex_to_lab(uint32_t hex) {
    const float scale = 1.0f / 65535.0f;
    float r = shore_srgb_linear_q16[(hex >> 16) & 0xffu] * scale;
    float g = shore_srgb_linear_q16[(hex >>  8) & 0xffu] * scale;
    float b = shore_srgb_linear_q16[ hex        & 0xffu] * scale;
    float l = shore_unit_cbrt(0.4122214708f*r + 0.5363325363f*g + 0.0514459929f*b);
    float m = shore_unit_cbrt(0.2119034982f*r + 0.6806995451f*g + 0.1073969566f*b);
    float s = shore_unit_cbrt(0.0883024619f*r + 0.2817188376f*g + 0.6299787005f*b);
    return (shore_lab_t){
         0.2104542553f*l + 0.7936177850f*m - 0.0040720468f*s,
         1.9779984951f*l - 2.4285922050f*m + 0.4505937099f*s,
         0.0259040371f*l + 0.7827717662f*m - 0.8086757660f*s
    };
}

static shore_lab_t shore_base_local[SHORE_LOCAL_LEDS];
static bool shore_base_cache_ready = false;
static bool shore_base_cache_sea_side = false;

static shore_lab_t shore_base_at(uint8_t led, bool left) {
    bool sea_side = left == (SHORE_SEA_ON_LEFT != 0);
    if (!shore_base_cache_ready || sea_side != shore_base_cache_sea_side) {
        uint8_t table = sea_side ? 0 : 1;
        for (uint8_t i = 0; i < SHORE_LOCAL_LEDS; ++i)
            shore_base_local[i] = shore_hex_to_lab(shore_base_hex[table][i]);
        shore_base_cache_ready = true;
        shore_base_cache_sea_side = sea_side;
    }
    return shore_base_local[led];
}

// Preserve the interruptible OKLab language morphs. Base colour offsets
// disappear smoothly as the persistent mode changes to a language/flag.
static shore_lab_t shore_apply_base_palette(shore_lab_t colour, uint8_t led,
                                           bool left, float language_level) {
    float amount = 1.0f - language_level;
    if (amount <= 0.0f) return colour;
    shore_lab_t target = shore_base_at(led, left);
    return (shore_lab_t){
        colour.l + amount * (target.l - SHORE_CODE.l),
        colour.a + amount * (target.a - SHORE_CODE.a),
        colour.b + amount * (target.b - SHORE_CODE.b)
    };
}

// Analytic broad compact-support wave: no fluid grid, no expf or powf.
// All wave and breath motion is pure and time-addressable, even if frames skip.
static float shore_swell_at(float x, float y, float centre, float width) {
    float d = (x - centre + SHORE_WAVE_TILT * (y - 0.33f)) / width;
    float d2 = d*d;
    if (d2 >= 1.0f) return 0.0f;
    float soft = 1.0f - d2;
    return soft*soft;
}
// A deterministic, time-addressable sequence of *individual* swells.
// Each RP2040 derives the same pseudo-random sequence from the USB-primary's
// shared animation epoch. Only wave parameters, not per-LED frames, are shared.
// The sequence is cached and only advances when the current wave finishes.
typedef struct {
    uint32_t epoch;
    uint32_t seed;
    uint32_t start;
    uint32_t index;
    uint32_t formation_ms, travel_ms, settle_ms, calm_ms, total_ms;
    float sea_state, size, width, strength;
    bool valid;
} shore_wave_t;

static shore_wave_t shore_wave = {0};

// 32-bit integer avalanche: identical on both halves, independent of libc PRNG.
static uint32_t shore_hash32(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    return x ^ (x >> 16);
}

static float shore_noise(uint32_t seed, uint32_t wave_index, uint32_t salt) {
    uint32_t h = shore_hash32(seed ^ (wave_index * 0x9e3779b9u) ^ salt);
    return (float)(h >> 16) / 65535.0f;
}

static uint32_t shore_uint_between(float t, uint32_t lo, uint32_t hi) {
    return lo + (uint32_t)((float)(hi - lo) * shore_clamp(t, 0.0f, 1.0f) + 0.5f);
}

static void shore_generate_wave(shore_wave_t *w, bool first) {
    float sea_target = shore_noise(w->seed, w->index, 0x13ac6f3du);
    w->sea_state = first ? sea_target
        : SHORE_V2_SEA_STATE_MEMORY * w->sea_state
        + (1.0f - SHORE_V2_SEA_STATE_MEMORY) * sea_target;

    // Closely spaced waves have similar scale; sea state drifts over multiple
    // waves. The independent term keeps individual waves unpredictable.
    float independent = shore_noise(w->seed, w->index, 0xb087c21fu);
    float target_size = 0.70f * w->sea_state + 0.30f * independent;
    if (first) {
        w->size = target_size;
    } else {
        w->size += shore_clamp(target_size - w->size,
                              -SHORE_V2_SIZE_SLEW, SHORE_V2_SIZE_SLEW);
    }
    w->size = shore_clamp(w->size, 0.0f, 1.0f);

    w->strength = SHORE_WAVE_STRENGTH * (0.52f + 0.48f * w->size);
    w->width = SHORE_WAVE_HALF_WIDTH * (0.90f + 0.20f * w->size);
    w->formation_ms = shore_uint_between(
        shore_noise(w->seed, w->index, 0x19994e21u),
        SHORE_V2_FORMATION_MIN_MS, SHORE_V2_FORMATION_MAX_MS);

    // Long-period/large swells generally move more quickly in this visual
    // model. Duration is not an independent random quantity.
    float travel = (float)SHORE_V2_TRAVEL_SLOW_MS
        + ((float)SHORE_V2_TRAVEL_FAST_MS - (float)SHORE_V2_TRAVEL_SLOW_MS) * w->size;
    travel += (shore_noise(w->seed, w->index, 0xe871ca42u) - 0.5f)
        * (float)SHORE_V2_TRAVEL_JITTER_MS;
    w->travel_ms = (uint32_t)(travel + 0.5f);
    w->settle_ms = shore_uint_between(w->size,
        SHORE_V2_SETTLE_SMALL_MS, SHORE_V2_SETTLE_LARGE_MS);

    // A more energetic sea tends to have shorter rests, with genuine random
    // variation remaining between waves. Every rest is still visibly calm.
    float calm_noise = shore_noise(w->seed, w->index, 0x475e2f5bu);
    float calm_mix = shore_clamp(0.73f * calm_noise + 0.27f * (1.0f - w->sea_state), 0.0f, 1.0f);
    w->calm_ms = shore_uint_between(calm_mix, SHORE_V2_CALM_MIN_MS, SHORE_V2_CALM_MAX_MS);
    w->total_ms = w->formation_ms + w->travel_ms + w->settle_ms + w->calm_ms;
}

static void shore_wave_prepare(uint32_t now, uint32_t active_epoch) {
    if (!shore_wave.valid || shore_wave.epoch != active_epoch) {
        shore_wave = (shore_wave_t){
            .epoch = active_epoch,
            .seed = shore_hash32(active_epoch ^ 0xa9643f2bu),
            .start = active_epoch + SHORE_ENTRANCE_BLEND_MS,
            .index = 0,
            .valid = true
        };
        shore_generate_wave(&shore_wave, true);
    }

    // Usually zero iterations; at most one new wave every ~25-35 seconds.
    // A reconnect after a long absence catches up deterministically.
    while ((int32_t)(now - shore_wave.start) >= (int32_t)shore_wave.total_ms) {
        shore_wave.start += shore_wave.total_ms;
        ++shore_wave.index;
        shore_generate_wave(&shore_wave, false);
    }
}

typedef enum { SHORE_FORMING, SHORE_ADVANCING, SHORE_SETTLING, SHORE_CALM } shore_stage_t;
typedef struct {
    shore_stage_t stage;
    float progress; // 0..1 within stage
    float centre;
    float activity; // 1 during advancing, common fade during settling
} shore_scene_t;

static shore_scene_t shore_scene_at(uint32_t now) {
    shore_scene_t scene = {SHORE_CALM, 0.0f, 0.0f, 0.0f};
    int32_t elapsed = (int32_t)(now - shore_wave.start);
    if (elapsed < 0) return scene; // entrance choreography is still running
    uint32_t t = (uint32_t)elapsed;
    if (t < shore_wave.formation_ms) {
        scene.stage = SHORE_FORMING;
        scene.progress = shore_smooth((float)t / (float)shore_wave.formation_ms);
        scene.activity = scene.progress;
    } else if ((t -= shore_wave.formation_ms) < shore_wave.travel_ms) {
        scene.stage = SHORE_ADVANCING;
        scene.progress = (float)t / (float)shore_wave.travel_ms;
        scene.activity = 1.0f;
    } else if ((t -= shore_wave.travel_ms) < shore_wave.settle_ms) {
        scene.stage = SHORE_SETTLING;
        scene.progress = shore_smooth((float)t / (float)shore_wave.settle_ms);
        scene.activity = 1.0f - scene.progress;
    } else {
        scene.stage = SHORE_CALM;
        scene.progress = 1.0f;
        scene.activity = 0.0f;
    }

    float travel = 0.0f;
    if (scene.stage == SHORE_ADVANCING) {
        float p = scene.progress;
        const float ramp = SHORE_V2_EDGE_RAMP;
        if (p < ramp) {
            travel = (p * p) / (2.0f * ramp * (1.0f - ramp));
        } else if (p > 1.0f - ramp) {
            float tail = 1.0f - p;
            travel = 1.0f - (tail * tail) / (2.0f * ramp * (1.0f - ramp));
        } else {
            travel = (p - 0.5f * ramp) / (1.0f - ramp);
        }
    } else if (scene.stage == SHORE_SETTLING) {
        travel = 1.0f;
    }
    scene.centre = -SHORE_WAVE_OVERSHOOT + shore_total_width() * travel;
    return scene;
}

static float shore_normal_field(float x, float y,
                               const shore_scene_t *scene, uint32_t now,
                               float *chroma_ripple,
                               float *hue_ripple) {
    *chroma_ripple = 0.0f;
    *hue_ripple = 0.0f;
    float wave = 0.0f;
    if (scene->activity > 0.0f) {
        float crest = shore_wave.strength *
            shore_swell_at(x, y, scene->centre, shore_wave.width);
        float at_break = scene->stage == SHORE_FORMING ? 0.0f
            : (scene->stage == SHORE_ADVANCING
                ? shore_smooth((scene->progress - 0.72f) / 0.28f) : 1.0f);
        float wash = SHORE_V2_BREAK_WASH * shore_wave.strength * at_break;
        wave = (crest + (1.0f - crest) * wash) * scene->activity;
    }

    // Spatial ripple is present during active surf but *disappears on the same
    // clock tick everywhere* during settling. In calm, only a faint shared
    // breathing signal remains; sea and beach have different fixed baselines.
    float ripple = 0.0f;

    if (scene->activity > 0.0f) {
        uint32_t offset = (uint32_t)(
            (shore_total_width() - x) * SHORE_UNDERCURRENT_LAG_MS
        );

        uint32_t phase_time = now + offset;

        ripple = 0.5f + 0.5f *
            shore_sin_ms(phase_time, SHORE_UNDERCURRENT_PERIOD_MS);

        // Brighter ripple: slightly less saturated.
        *chroma_ripple = scene->activity * (1.0f - 2.0f * ripple);

        // Same travelling phase, offset by 90 degrees for hue movement.
        *hue_ripple = scene->activity * shore_sin_ms(
            phase_time + SHORE_UNDERCURRENT_PERIOD_MS / 4u,
            SHORE_UNDERCURRENT_PERIOD_MS
        );
    }
    float common_breath = 0.5f + 0.5f * shore_sin_ms(now, 109000u);
    float beach_scale = 1.0f + (SHORE_BEACH_AMBIENT_SCALE - 1.0f) * shore_beachness(x);
    float active_ambient = scene->activity * beach_scale *
        (SHORE_UNDERCURRENT_STRENGTH * ripple + SHORE_BREATH_STRENGTH * common_breath);
    float calm_ambient = SHORE_V2_CALM_BREATH * common_breath;
    float ambient = active_ambient + calm_ambient;
    return shore_clamp(wave + (1.0f - wave) * ambient, 0.0f, 1.0f);
}

// The arrival is a light field *plus a separate visibility/coverage mask*.
// In v1/v2, deriving coverage as incoming_field / normal_field saturated at
// almost zero illumination during v2's calm, visibly switching LEDs on in
// columns. Coverage must not depend on the ambient field's brightness.
typedef struct {
    float field;
    float coverage;
} shore_arrival_t;

static shore_arrival_t shore_arrival_at(float x, float y, uint32_t elapsed,
                                      float normal) {
    if (elapsed >= SHORE_ENTRANCE_END_MS)
        return (shore_arrival_t){normal, 1.0f};

    if (elapsed < SHORE_ENTRANCE_RETREAT_MS) {
        // A small tentative wash appears at the sea edge, then withdraws.
        bool retreat = elapsed >= SHORE_ENTRANCE_FIRST_MS;
        float p = retreat
            ? (float)(elapsed - SHORE_ENTRANCE_FIRST_MS) /
              (float)(SHORE_ENTRANCE_RETREAT_MS - SHORE_ENTRANCE_FIRST_MS)
            : (float)elapsed / (float)SHORE_ENTRANCE_FIRST_MS;
        float travel = retreat ? 1.0f - shore_smooth(p) : shore_smooth(p);
        float centre = -0.70f + 1.22f * travel;
        float swell = shore_swell_at(x, y, centre, 0.62f);
        return (shore_arrival_t){
            .field = 0.43f * swell,
            .coverage = shore_clamp(SHORE_FIRST_WAVE_OPACITY * swell, 0.0f, 1.0f)
        };
    }

    // Second, stronger wave travels over the *whole* keyboard, including
    // the unlit space between halves. A broad, slightly tilted front gives
    // each LED time to grow from dark to full illumination.
    float p = (float)(elapsed - SHORE_ENTRANCE_RETREAT_MS) /
              (float)(SHORE_ENTRANCE_END_MS - SHORE_ENTRANCE_RETREAT_MS);
    float front = -0.55f + (shore_total_width() + 1.2f) * shore_smooth(p);
    float tilted_x = x + SHORE_ENTRANCE_TILT * (y - 0.33f);
    float coverage = shore_smooth(
        (front - tilted_x + SHORE_ENTRANCE_LEAD) / SHORE_ENTRANCE_FEATHER);
    float incoming = 0.20f +
        0.60f * shore_swell_at(x, y, front, SHORE_WAVE_HALF_WIDTH) +
        0.30f * normal;

    // Blend BOTH illumination and visibility into the ongoing Shore field.
    // At the end we have exactly {normal, 1}, with no sudden jump.
    if (elapsed >= SHORE_ENTRANCE_BLEND_MS) {
        float mix = shore_smooth((float)(elapsed - SHORE_ENTRANCE_BLEND_MS) /
                                 (float)(SHORE_ENTRANCE_END_MS - SHORE_ENTRANCE_BLEND_MS));
        incoming += (normal - incoming) * mix;
        coverage += (1.0f - coverage) * mix;
    }
    return (shore_arrival_t){incoming, coverage};
}

static void shore_set_dark(void) {
    if (!shore_last_output_dark) {
        rgblight_driver.set_color_all(0, 0, 0);
        rgblight_driver.flush();
        shore_last_output_dark = true;
    }
}

static void shore_slave_callback(uint8_t in_size, const void *in,
                                  uint8_t out_size, void *out) {
    (void)out_size; (void)out;
    if (in_size == sizeof(shore_packet_t) && in != NULL) {
        shore_remote = *(const shore_packet_t *)in;
        shore_remote_received_at = timer_read32(); // local monotonic timeout
    }
}

static void shore_init(void) {
    shore_inited = true;
    // Do not trust an EEPROM-saved Vial rainbow state.
    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
    rgblight_driver.set_color_all(0, 0, 0);
    rgblight_driver.flush();
    shore_last_output_dark = true;
    shore_last_frame = timer_read32();
    transaction_register_rpc(SHORE_SYNC, shore_slave_callback);
}

static void shore_primary_update(uint32_t now) {
    bool configured = (usb_device_state_get_configure_state() == USB_DEVICE_STATE_CONFIGURED);
    if (configured && !shore_master_active) {
        // Brief re-enumeration preserves epoch; a genuinely long sleep starts
        // a new arrival. This threshold is a policy, not a USB guarantee.
        if (!shore_ever_active || now - shore_last_active_at > 3000u) shore_epoch = now;
        shore_ever_active = true;
    }
    shore_master_active = configured;
    if (configured) shore_last_active_at = now;

    bool changed = (shore_remote.active != (uint8_t)configured) ||
                   (shore_remote.active_epoch != shore_epoch);
    if (changed || timer_elapsed32(shore_last_send) >= SHORE_SPLIT_SYNC_MS) {
        shore_packet_t packet = {shore_epoch, configured ? 1u : 0u};
        // No per-frame RPC; only a ~4 Hz heartbeat and immediate state changes.
        if (is_transport_connected()) {
            if (transaction_rpc_send(SHORE_SYNC, sizeof(packet), &packet)) {
                shore_last_send = timer_read32();
            }
        } else {
            shore_last_send = timer_read32(); // avoid blocking a standalone half
        }
        shore_remote = packet; // master renders locally even if the split is down
    }
}

static bool shore_host_active(void) {
    if (is_keyboard_master()) return shore_master_active;
    if (timer_elapsed32(shore_remote_received_at) > SHORE_SPLIT_TIMEOUT_MS) return false;
    return shore_remote.active != 0;
}

static void shore_render(uint32_t now, layer_state_t state, bool left, uint32_t active_epoch) {
    shore_lab_t base, centre, top;
    float top_emphasis, language_level;
    shore_palette_sample(now, state, left, &base, &centre, &top, &top_emphasis, &language_level);
    float under_min_sea =
        SHORE_CODER_MIN_SEA +
        language_level * (SHORE_LANGUAGE_MIN_SEA - SHORE_CODER_MIN_SEA);

    float under_min_beach =
        SHORE_CODER_MIN_BEACH +
        language_level * (SHORE_LANGUAGE_MIN_BEACH - SHORE_CODER_MIN_BEACH);

    float under_max =
        SHORE_CODER_MAX +
        language_level * (SHORE_LANGUAGE_MAX - SHORE_CODER_MAX);
    float top_min = SHORE_TOP_AMBIENT_MIN + top_emphasis*(SHORE_TOP_ACTIVE_MIN-SHORE_TOP_AMBIENT_MIN);
    float top_max = SHORE_TOP_AMBIENT_MAX + top_emphasis*(SHORE_TOP_ACTIVE_MAX-SHORE_TOP_AMBIENT_MAX);
    uint32_t elapsed = now - active_epoch;
    // Wave dynamics calculated once/frame. Base colours may differ per LED.
    shore_wave_prepare(now, active_epoch);
    shore_scene_t scene = shore_scene_at(now);

    for (uint8_t i = 0; i < SHORE_LOCAL_LEDS; i++) {
        float x = shore_from_sea(shore_u[i], left);
        float chroma_ripple = 0.0f;
        float hue_ripple = 0.0f;

        float normal = shore_normal_field(
            x, shore_v[i], &scene, now,
            &chroma_ripple, &hue_ripple
        );
        shore_arrival_t arrival = shore_arrival_at(x, shore_v[i], elapsed, normal);
        float f = arrival.field;
        bool is_top = i == 6;

        float beachness = shore_beachness(x);
        float under_min =
            under_min_sea +
            (under_min_beach - under_min_sea) * beachness;

        float min = is_top ? top_min : under_min;
        float max = is_top ? top_max : under_max;
        // Apply the physical exposure mask to the entire brightness envelope,
        // including the baseline; no LED is abruptly switched to its minimum.
        float level = (min + (max-min)*shore_clamp(f,0.0f,1.0f)) * arrival.coverage;
        if (is_top) level = shore_clamp(level,0.0f,SHORE_TOP_OUTPUT_CAP);
        uint8_t r,g,b;
        // LEDs 1/6 are white in French and Irish modes, including morphs.
        // LED 7 always uses the current language/base palette, even on NAV/SYM.
        shore_lab_t colour = is_top ? top : ((i == 0 || i == 5) ? centre : base);
        colour = shore_apply_base_palette(colour, i, left, language_level);

        // The same flag desaturation and gentle Shore-driven chromatic motion
        // apply whether or not a transient layer is held. Only brightness changes.
        float chroma = 1.0f - language_level * (1.0f - SHORE_FLAG_CHROMA);
        float chroma_scale = chroma * (1.0f + SHORE_CHROMA_MOTION * chroma_ripple);

        // Convert hue displacement from degrees to radians.
        float angle = SHORE_HUE_MOTION_DEG * 0.01745329252f * hue_ripple;

        // Small-angle rotation in the OKLab a/b plane.
        // Keep both original coordinates for the rotation.
        float a_ch = colour.a * chroma_scale;
        float b_ch = colour.b * chroma_scale;

        colour.a = a_ch - angle * b_ch;
        colour.b = b_ch + angle * a_ch;

        shore_rgb_scaled(shore_linear_from_lab(colour), level, &r,&g,&b);
        rgblight_driver.set_color(i,r,g,b); // BUFFER ONLY, no flush
    }
    rgblight_driver.flush();               // exactly one WS2812 transfer / half / frame
    shore_last_output_dark = false;
}

static void shore_task(layer_state_t state) {
    if (!shore_inited) return;
    uint32_t now = sync_timer_read32();
    if (is_keyboard_master()) shore_primary_update(now);
    if (!shore_host_active()) {
        shore_set_dark();
        return;
    }
    // This is a bound, not a frame-count-driven simulation step.
    if (timer_elapsed32(shore_last_frame) < SHORE_FRAME_MS) return;
    shore_last_frame = timer_read32();
    shore_render(now, state, is_keyboard_left(),
                 is_keyboard_master() ? shore_epoch : shore_remote.active_epoch);
}

// This is intentionally NOT a graceful animated departure. True bus suspend
// cannot be treated as a 1–3 s shutdown warning.
static void shore_suspend(void) {
    shore_master_active = false;
    shore_remote.active = 0;
    shore_set_dark();
    // One best-effort inactive notification while the split transport is still
    // available. Not a guaranteed 2.5 mA USB-suspend-compliant solution:
    // actual peripheral idle consumption must be measured on the hardware.
    if (is_keyboard_master() && is_transport_connected()) {
        shore_packet_t dark = {shore_epoch, 0};
        transaction_rpc_send(SHORE_SYNC, sizeof(dark), &dark);
    }
}
