// Copyright 2025 Martin Raspaud (@mraspaud)
// SPDX-License-Identifier: GPL-2.0
#include QMK_KEYBOARD_H
#include "keymap_extras/keymap_eurkey.h"
#define QU_TIMEOUT 1000
static uint16_t q_timer = 0;
// Set after typing ':' until the end of the word, so that vim commands like :qa
// or :wqa don't get the automatic 'u' after 'q'.
static bool in_colon_cmd = false;
static uint16_t last_keycode_time = 0;
static uint16_t sym_layer_timer = 0;

// Layers declarations
enum {
    L_BASE = 0,
    L_EN,
    L_TH,
    L_SE,
    L_FR,
    L_NUMSYM,
    L_NAV,
    L_FRSYM,
    L_FN,
};

#define HSV_SE_BLUE    143, 255, 167   // #006AA7
#define HSV_SE_YELLOW   34, 255, 255   // #FFCD00

#define HSV_FR_BLUE    170, 255, 145   // #000091
#define HSV_FR_RED     252, 255, 225   // #E1000F

#define HSV_IE_GREEN   98, 255, 169   // #169B62
#define HSV_IE_ORANGE  20, 255, 255   // #FF883E

#define HSV_KANAGAWA_SYMBOL  186,  79, 184   // oniViolet  #957FB8
#define HSV_KANAGAWA_NAV     140,  95, 202   // springBlue #7FB4CA
#define HSV_KANAGAWA_CODE 119, 70, 120   // waveAqua2, deliberately dim

#define TOP_LED_BRIGHTNESS 80

#define GLOW_INTERVAL_MS 20

// 94 steps every 20 ms => one complete cycle in ~14 seconds.
#define GLOW_PHASE_STEP 65

// Bottom of the wave: 160/255 = 63% brightness.
#define GLOW_MIN_SCALE (160u * 257u)

// Smooth cosine-like cycle, 0..65535.
static const uint16_t glow_curve[16] = {
        0,  2495,  9598, 20228,
    32768, 45307, 55938, 63041,
    65535, 63041, 55938, 45307,
    32768, 20228,  9598,  2495
};

static uint16_t glow_shape(uint16_t phase) {
    uint8_t index = phase >> 12;
    uint16_t fraction = phase & 0x0FFF;

    int32_t a = glow_curve[index];
    int32_t b = glow_curve[(index + 1) & 0x0F];

    return a + (((b - a) * fraction) >> 12);
}

static uint8_t glow_value(uint8_t base_v, uint16_t phase, bool coder) {
    uint16_t shape = glow_shape(phase);

    // Coder: 75–100% of its already-dim colour.
    // Languages: 45–100%, so the travelling glow is obvious.
    uint16_t min_scale = coder ? 49152u : 29491u;

    uint32_t scale =
        min_scale +
        (((uint32_t)(65535u - min_scale) * shape) >> 16);

    return ((uint32_t)base_v * scale) >> 16;
}


static hsv_t persistent_colour(bool left) {
    if (layer_state_cmp(layer_state, L_SE)) {
        return left ? (hsv_t){HSV_SE_BLUE}
                    : (hsv_t){HSV_SE_YELLOW};
    }

    if (layer_state_cmp(layer_state, L_FR)) {
        return left ? (hsv_t){HSV_FR_BLUE}
                    : (hsv_t){HSV_FR_RED};
    }

    if (layer_state_cmp(layer_state, L_EN)) {
        return left ? (hsv_t){HSV_IE_GREEN}
                    : (hsv_t){HSV_IE_ORANGE};
    }

    // Coder/base layer.
    return (hsv_t){HSV_KANAGAWA_CODE};
}

static hsv_t transient_colour(void) {
    if (layer_state_cmp(layer_state, L_NUMSYM)) {
        return (hsv_t){
            186, 79, TOP_LED_BRIGHTNESS   // oniViolet #957FB8
        };
    }

    if (layer_state_cmp(layer_state, L_NAV)) {
        return (hsv_t){
            141, 81, TOP_LED_BRIGHTNESS
        };
    }

    return (hsv_t){0, 0, 0};
}


enum unicode_names {
    ACRC,
    AACRC,
    ECRC,
    EECRC,
    ICRC,
    IICRC,
    OCRC,
    OOCRC,
    UCRC,
    UUCRC,
    NDASH,
    MDASH,
    MINUS,
    HYPHEN,
    MIDPOINT,
    LANGL,
    RANGL,
    SEQL,
    GEQL,
    ITBG,
};

const uint32_t PROGMEM unicode_map[] = {
    [ACRC] = 0x00E2,  // â
    [AACRC] = 0x00C2,  // Â
    [ECRC] = 0x00EA,  // ê
    [EECRC] = 0x00CA,  // Ê
    [ICRC] = 0x00EE,  // î
    [IICRC] = 0x00CE,  // Î
    [OCRC] = 0x00F4,  // ô
    [OOCRC] = 0x00D4,  // Ô
    [UCRC] = 0x00FB,  // û
    [UUCRC] = 0x00DB,  // Û
    [NDASH] = 0x2013,  // –
    [MDASH] = 0x2014,  // —
    [MINUS] = 0x2212,  // −
    [HYPHEN] = 0x002D,  // -
    [LANGL] = 0x27E8, // ⟨
    [RANGL] = 0x27E9, // ⟩
    [SEQL] = 0x2264, // ≤
    [GEQL] = 0x2265, // ≥
    [ITBG] = 0x203D, // ‽
    [MIDPOINT] = 0x00B7, // ·
};

#define U_ACRC UP(ACRC, AACRC)
#define U_ECRC UP(ECRC, EECRC)
#define U_ICRC UP(ICRC, IICRC)
#define U_OCRC UP(OCRC, OOCRC)
#define U_UCRC UP(UCRC, UUCRC)
#define U_DASH UP(NDASH, MDASH)
#define U_HYPHEN UP(NDASH, HYPHEN)
#define U_MINUS UP(MINUS, MDASH)
#define U_LANGL UP(LANGL, SEQL)
#define U_RANGL UP(RANGL, GEQL)
#define U_BANGD UP(ITBG, MIDPOINT)
#define KC_PSMS S(EU_DGRK)
#define MAGICFR OSL(L_FRSYM)

enum custom_keycodes {
    CKC_OU = SAFE_RANGE,
    DI_TH,
    CKC_TLD,
};

// Tap Dance declarations
enum {
    TD_ADIA,
    TD_ODIA,
    TD_CURR,
    TD_THRN,
};

// Tap Dance definitions
tap_dance_action_t tap_dance_actions[] = {
    // Tap once for Escape, twice for Caps Lock
    [TD_ADIA] = ACTION_TAP_DANCE_DOUBLE(EU_ADIA, EU_AE),
    [TD_ODIA] = ACTION_TAP_DANCE_DOUBLE(EU_ODIA, EU_OSTR),
    [TD_CURR] = ACTION_TAP_DANCE_DOUBLE(EU_EURO, EU_PND),
    // [TD_THRN] = ACTION_TAP_DANCE_DOUBLE(DI_TH, EU_THRN),
};

#define SE_ODIA TD(TD_ODIA)
#define SE_ADIA TD(TD_ADIA)
#define SN_CURR TD(TD_CURR)
// #define BA_THRN TD(TD_THRN)
#define OS_LSFT OSM(MOD_LSFT)
#define OS_LALT OSM(MOD_LALT)
#define OS_LCTL OSM(MOD_LCTL)
#define LT_SPC LT(L_NAV, KC_SPC)
#define LT_R LT(L_NUMSYM, KC_R)
#define MT_ESC LALT_T(KC_ESC)
#define CK_NBSP RALT(KC_SPC)
#define CK_NNBS S(RALT(KC_SPC))
#define MT_E RSFT_T(KC_E)
#define MT_N LSFT_T(KC_N)
#define MT_M RCTL_T(KC_M)
#define MT_S LCTL_T(KC_S)
#define MT_0 RSFT_T(KC_0)
#define MT_1 LSFT_T(KC_1)
#define MT_DGRV LSFT_T(EU_DGRV)


bool is_shift_pressed(keyrecord_t *record) {
    const uint8_t saved_mods = get_mods();
#ifndef NO_ACTION_ONESHOT
    const uint8_t mods = saved_mods | get_oneshot_mods();
#else
    const uint8_t mods = saved_mods;
#endif  // NO_ACTION_ONESHOT
#if CUSTOM_SHIFT_KEYS_LAYER_MASK != 0
    const uint8_t layer = read_source_layers_cache(record->event.key);
#endif  // CUSTOM_SHIFT_KEYS_LAYER_MASK
    if ((mods & MOD_MASK_SHIFT) != 0  // Shift is held.
#if CUSTOM_SHIFT_KEYS_NEGMODS != 0
        // Nothing in CUSTOM_SHIFT_KEYS_NEGMODS is held.
        && (mods & (CUSTOM_SHIFT_KEYS_NEGMODS)) == 0
#endif  // CUSTOM_SHIFT_KEYS_NEGMODS != 0
#if CUSTOM_SHIFT_KEYS_LAYER_MASK != 0
        // Pressed key is on a layer appearing in the layer mask.
        && ((1 << layer) & (CUSTOM_SHIFT_KEYS_LAYER_MASK)) != 0
#endif  // CUSTOM_SHIFT_KEYS_LAYER_MASK
    ) {
        return true;
    } else {
        return false;
    }
}

void send_unshifted_string(const char *string) {
    // Send a string without changing capitalisation.
    const uint8_t saved_mods = get_mods();
#ifndef NO_ACTION_ONESHOT
    const uint8_t mods = saved_mods | get_weak_mods() | get_oneshot_mods();
#else
    const uint8_t mods = saved_mods | get_weak_mods();
#endif  // NO_ACTION_ONESHOT

    // cancel shift mods, press the key, and restore mods.
    del_weak_mods(MOD_MASK_SHIFT);
#ifndef NO_ACTION_ONESHOT
    del_oneshot_mods(MOD_MASK_SHIFT);
#endif  // NO_ACTION_ONESHOT
    unregister_mods(MOD_MASK_SHIFT);
    set_oneshot_mods(MOD_BIT(KC_LSFT));
    SEND_STRING(string);
    set_mods(mods);
}


bool caps_word_press_user(uint16_t keycode) {
    switch (keycode) {
        // Keycodes that continue Caps Word, with shift applied.
        case KC_A ... KC_Z:
        case KC_MINS:
        case DI_TH:
            add_weak_mods(MOD_BIT(KC_LSFT));  // Apply shift to next key.
            return true;

        // Keycodes that continue Caps Word, without shifting.
        case KC_1 ... KC_0:
        case KC_BSPC:
        case KC_DEL:
        case KC_UNDS:
            return true;

        default:
            return false;  // Deactivate Caps Word.
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Check if the 'qu' timer is currently active
    if (q_timer != 0) {
        // If more time has passed than our timeout, cancel the 'qu' action
        if (timer_elapsed(q_timer) > QU_TIMEOUT) {
            q_timer = 0;
        }
    }

    switch (keycode) {
        case CKC_TLD:
            // Send tilde directly. This is to avoid having ~/ become ~* (the shift in tilde bleeds into / otherwise).
            if (record->event.pressed) {
                tap_code16(EU_TILD);
            }
            return false;
        case CKC_OU:
            if (record->event.pressed) {
                if (is_shift_pressed(record)) {
                    SEND_STRING("O");
                } else {
                    SEND_STRING("o");
                }
                send_unicode_string("ù");
            } else {
                // when keycode QMKBEST is released
            }
            break;
        case DI_TH:
            if (record->event.pressed) {
                if (is_caps_word_on()) {
                    SEND_STRING("TH");
                } else if (is_shift_pressed(record)) {
                    send_unshifted_string("Th");
                } else {
                    SEND_STRING("th");
                }
            } else {
                // key release
            }
            break;
        case KC_Q:
            if (record->event.pressed) {
                if (!in_colon_cmd) {
                    q_timer = timer_read();
                }
            }
            break;
        case KC_A:
        case MT_E:
        case KC_I:
        case KC_O:
        case KC_Y:
        case EU_EACU:
        case U_ACRC:
        case U_ECRC:
        case EU_EGRV:
        case U_ICRC:
        case EU_QUOT:
        case EU_RSQU:
            if (record->event.pressed) {
                if (q_timer != 0) {
                    q_timer = 0;
                    if (is_caps_word_on()) {
                        SEND_STRING("U");
                    } else {
                        tap_code(KC_U);
                    }
                }
            }
            break;
        case MAGICFR:
            break;
        default:
            if (record->event.pressed) {
                q_timer = 0;
            }
            break;
    }
    if (record->event.pressed) {
        switch (keycode) {
            case EU_COLN:
                in_colon_cmd = true;
                break;
            case LT_SPC:
            case KC_SPC:
            case KC_ENT:
            case MT_ESC:
            case KC_ESC:
                in_colon_cmd = false;
                break;
        }
        // Correcting with backspace is not typing: it is usually what happens
        // just before toggling a layer, so it must not delay the layer combos.
        switch (keycode) {
            case KC_BSPC:
            case KC_DEL:
                break;
            default:
                last_keycode_time = record->event.time;
                break;
        }
    }
    // your code here
    return true;
}

uint16_t get_flow_tap_term(uint16_t keycode, keyrecord_t* record,
                           uint16_t prev_keycode) {
    if (is_flow_tap_key(keycode) && is_flow_tap_key(prev_keycode)) {
        switch (keycode) {
            case MT_E:
            case MT_N:
            case LT_R:
            case LT_SPC:
              return 0;  // Short timeout on these keys.

            default:
              return FLOW_TAP_TERM;  // Longer timeout otherwise.
        }
    }
    return 0;  // Disable Flow Tap.
}

void keyboard_post_init_user(void) {
    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
    rgblight_setrgb(0, 0, 0);
}


 const custom_shift_key_t custom_shift_keys[] = {
  {KC_DOT , KC_EXLM}, // Shift . is !
  {KC_COMM, KC_QUES}, // Shift , is :
  {KC_COLN, KC_SCLN}, // Shift : is ;
  {EU_DQUO, EU_GRV }, // Shift " is `
  {KC_UNDS, KC_MINS }, // Shift _ is -
  {KC_LBRC, KC_LCBR }, // Shift [ is {
  {KC_RBRC, KC_RCBR }, // Shift ] is }
  {KC_LPRN, KC_LABK }, // Shift ( is <
  {KC_RPRN, KC_RABK }, // Shift } is >
  {KC_SLSH, EU_ASTR }, // Shift / is *
  {EU_QUOT, EU_RSQU }, // Shift ' is ’
  {KC_EQL, KC_PLUS}, // Shift = is +
  {EU_AT, EU_HASH}, // Shift @ is #
  {EU_RSQU, EU_RDQU}, // Shift ‘ is “
  {EU_LSQU, EU_LDQU}, // Shift ’ is ”
  {EU_RDAQ, EU_RDQU}, // Shift « is “
  {EU_LDAQ, EU_LDQU}, // Shift » is ”
  {EU_DEG, EU_IQUE}, // Shift ° is ¿
  {CK_NNBS, CK_NBSP},
  {KC_BSPC, KC_DELETE},
  {EU_ELLP, EU_MDDT}, // Shift … is ·

};


const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] PROGMEM =
    LAYOUT_split_3x6_3(
        'L', 'L', 'L', 'L', 'L', 'L',  'R', 'R', 'R', 'R', 'R', 'R',
        'L', 'L', 'L', 'L', 'L', 'L',  'R', 'R', 'R', 'R', 'R', 'R',
        'L', 'L', 'L', 'L', 'L', 'L',  'R', 'R', 'R', 'R', 'R', 'R',
                       '*', '*', '*',  '*', '*', '*'
    );


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
     /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ B │ W │ H │ G │ " │       │ : │ . │ / │ J │ X │ @ │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ S │ C │ N │ T │ K │       │ , │ A │ E │ I │ M │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ [ │ F │ P │ L │ D │ V │       │ = │ U │ O │ Y │ þ │ ] │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │ Qu├───┐           ┌───┤ ' │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ _ ├───┘
      *                       └───┘   └───┘
      */
    [L_BASE] = LAYOUT_split_3x6_3(
        KC_Z,    KC_B,    KC_W,    KC_H,    KC_G,    EU_DQUO,                                      EU_COLN, KC_DOT,  KC_SLSH, KC_J,    KC_X,    EU_AT,
        KC_LPRN, MT_S,    KC_C,    MT_N,    KC_T,    KC_K,                                         KC_COMM, KC_A,    MT_E,    KC_I,    MT_M,    KC_RPRN,
        KC_LBRC, KC_F,    KC_P,    KC_L,    KC_D,    KC_V,                                         KC_EQL,  KC_U,    KC_O,    KC_Y,    DI_TH,   KC_RBRC,
                                            KC_Q,   LT_R,  MT_ESC,                       KC_UNDS, LT_SPC,  EU_QUOT
    ),
     /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ B │ W │ H │ G │ " │       │ : │ . │ / │ J │ X │ @ │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ S │ C │ N │ T │ K │       │ , │ A │ E │ I │ M │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ “ │ F │ P │ L │ D │ V │       │ = │ U │ O │ Y │ þ │ ” │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │Qu ├───┐           ┌───┤ ’ │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ - ├───┘
      *                       └───┘   └───┘
      */
    [L_EN] = LAYOUT_split_3x6_3(
        _______, _______, _______, _______, _______, _______,                                      _______, _______, _______, _______, _______,  _______,
        _______, _______, _______, _______, _______, _______,                                      _______, _______, _______, _______, _______,  _______,
        EU_LSQU, _______, _______, _______, _______, _______,                                      _______, _______, _______, _______, _______,  EU_RSQU,
                                            _______, _______, _______,                    KC_UNDS, _______, EU_RSQU
    ),
      /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ W │ B │ H │ G │ ” │       │ : │ . │ Ä │ J │ X │ @ │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ C │ S │ N │ T │ K │       │ , │ A │ E │ I │ M │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ” │ P │ F │ L │ D │ V │       │ Å │ U │ O │ Y │ Ö │ ” │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │Qu ├───┐           ┌───┤ ’ │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ - ├───┘
      *                       └───┘   └───┘
      */
    [L_SE] = LAYOUT_split_3x6_3(
        _______, _______, _______, _______, _______, _______,                                      _______, _______, SE_ADIA, _______, _______,  _______,
        _______, _______, _______, _______, _______, _______,                                      _______, _______, _______, _______, _______,  _______,
        EU_RDQU, _______, _______, _______, _______, _______,                                      EU_ARNG, _______, _______, _______, SE_ODIA,  EU_RDQU,
                                            _______, _______, _______,                    KC_UNDS, _______, EU_RSQU
    ),
      /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ Ç │ B │ H │ G │ " │       │ : │ . │ É │ J │ X │ @ │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ C │ S │ N │ T │MgK│       │ , │ A │ E │ I │ M │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ « │ P │ F │ L │ D │ V │       │ À │ U │ O │ Y │ þ │ » │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │Qu ├───┐           ┌───┤ ’ │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ - ├───┘
      *                       └───┘   └───┘
      */
    [L_FR] = LAYOUT_split_3x6_3(
        _______, _______, EU_CCED, _______, _______, _______,                                      _______, _______, EU_EACU, _______, _______, _______,
        _______, _______, _______, _______, _______, MAGICFR,                                      _______, _______, _______, _______, _______, _______,
        EU_LDAQ, _______, _______, _______, _______, _______,                                      EU_AGRV, _______, _______, _______, EU_EGRV, EU_RDAQ,
                                            _______, _______, _______,                    KC_UNDS, _______, EU_RSQU
    ),
        /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │cps│ \ │ | │ & │ % │ " │       │ : │ . │ / │ ~ │ µ │ ° │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ 6 │ 4 │ 0 │ 2 │ – │       │ , │ 3 │ 1 │ 5 │ 7 │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ [ │ ^ │ € │ § │ 8 │ $ │       │ = │ 9 │ … │ · │ þ │ ] │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │Qu ├───┐           ┌───┤Lck│
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ _ ├───┘
      *                       └───┘   └───┘
      */
    [L_NUMSYM] = LAYOUT_split_3x6_3(
        KC_PSMS, KC_BSLS, KC_PIPE, KC_AMPR, KC_PERC, _______,                                      _______, _______, KC_SLSH, CKC_TLD, EU_DGRK, EU_DEG,
        _______, KC_6,    KC_4,    MT_0,    KC_2,    EU_NDSH,                                      _______, KC_3,    MT_1,    KC_5,    KC_7,    _______,
        KC_LBRC, EU_CIRC, SN_CURR, EU_SECT, KC_8,    KC_DLR,                                       _______, KC_9,    EU_ELLP, U_BANGD, EU_THRN, KC_RBRC,
                                            _______, _______, _______,                    KC_UNDS, CK_NNBS, QK_LLCK
    ),
       /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ W │ B │ H │ G │ " │       │brd│pdn│up │pup│bru│vup│
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │Gui│Alt│Sft│Ctl│ K │       │hom│lft│dn │rgt│end│vdn│
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ [ │ P │ F │ L │ D │ V │       │ = │prv│ply│nxt│stp│mut│
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │ Q ├───┐           ┌───┤ ' │
      *               └───┤Alt├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ _ ├───┘
      *                       └───┘   └───┘
      */
    [L_NAV] = LAYOUT_split_3x6_3(
        KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,                                        KC_BRID, KC_PGUP, KC_UP,   KC_PGDN, KC_BRIU, KC_VOLU,
        _______, OS_LCTL, KC_LGUI, OS_LSFT, OS_LALT, _______,                                      KC_HOME, KC_LEFT, KC_DOWN, KC_RGHT, KC_END,  KC_VOLD,
        KC_F7,   KC_F9,   KC_F9,   KC_F10,  KC_F11,  KC_F12,                                       _______, KC_MPRV, KC_MPLY, KC_MNXT, KC_MSTP, KC_MUTE,
                                            _______, _______, _______,                    _______, _______, _______
    ),
      /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ " │ B │ ' │ G │ " │       │ @ │ · │ È │ J │ X │ Q │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ « │ Ç │ ~ │ ` │ ^ │ K │       │ , │ Ê │ Ê │ Î │ µ │ » │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ [ │ P │ F │ L │ D │ V │       │ À │ Û │ Ô │Où │ Œ │ ] │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │Th ├───┐           ┌───┤Qu │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤ _ │   │Esc├───┘
      *                       └───┘   └───┘
      */
    [L_FRSYM] = LAYOUT_split_3x6_3(
        _______, _______, KC_W,    EU_ACUT, _______, EU_DIAE,                                      _______, RALT(KC_X), EU_EGRV, _______, _______, _______,
        _______, EU_CARN, EU_DTIL, MT_DGRV, EU_DCIR, KC_K,                                         _______, U_ACRC,  U_ECRC,  U_ICRC, _______, _______,
        _______, _______, _______, _______, _______, _______,                                      EU_UGRV, U_UCRC,  U_OCRC,  CKC_OU,  EU_OE,   _______,
                                            _______, _______, _______,                    _______, _______,   _______
    ),
};


const uint16_t PROGMEM combo_tab[] = {KC_C, KC_T, COMBO_END};
const uint16_t PROGMEM combo_backspace[] = {KC_SLASH, KC_J, COMBO_END};
const uint16_t PROGMEM combo_delete[] = {KC_X, KC_J, COMBO_END};
const uint16_t PROGMEM combo_backspace_se[] = {SE_ADIA, KC_J, COMBO_END};
const uint16_t PROGMEM combo_backspace_fr[] = {EU_EACU, KC_J, COMBO_END};
const uint16_t PROGMEM combo_backspace_sym[] = {KC_SLSH, CKC_TLD, COMBO_END};
const uint16_t PROGMEM combo_prtscr[] = {EU_COLN, KC_DOT, COMBO_END};
const uint16_t PROGMEM combo_capsword[] = {KC_LPRN, KC_RPRN, COMBO_END};
const uint16_t PROGMEM combo_bootloader[] = {EU_DQUO, EU_COLN, COMBO_END};
const uint16_t PROGMEM combo_kb_reboot[] = {EU_AT, KC_Z, COMBO_END};
const uint16_t PROGMEM combo_sleep[] = {KC_COMM, KC_EQL, COMBO_END};
const uint16_t PROGMEM combo_enter[] = {KC_A, KC_I, COMBO_END};
const uint16_t PROGMEM combo_en[] = {MT_N, MT_E, COMBO_END};
const uint16_t PROGMEM combo_se[] = {MT_S, MT_E, COMBO_END};
const uint16_t PROGMEM combo_fr[] = {KC_F, LT_R, COMBO_END};
const uint16_t PROGMEM combo_base[] = {KC_P, KC_L, KC_D, COMBO_END};
enum combo_index {
    CMB_TAB,
    CMB_BACKSPACE,
    CMB_DELETE,
    CMB_BACKSPACE_SE,
    CMB_BACKSPACE_FR,
    CMB_BACKSPACE_SYM,
    CMB_PRTSCR,
    CMB_CAPSWORD,
    CMB_BOOTLOADER,
    CMB_KB_REBOOT,
    CMB_SLEEP,
    CMB_ENTER,
    CMB_FR,
    CMB_SE,
    CMB_EN,
    CMB_BASE,
};

combo_t key_combos[] = {
    COMBO(combo_tab, KC_TAB),
    COMBO(combo_backspace, KC_BSPC),
    COMBO(combo_delete, KC_DEL),
    COMBO(combo_backspace_se, KC_BSPC),
    COMBO(combo_backspace_fr, KC_BSPC),
    COMBO(combo_backspace_sym, KC_BSPC),
    COMBO(combo_prtscr, KC_PRINT_SCREEN),
    COMBO(combo_capsword, CW_TOGG),
    COMBO(combo_bootloader, QK_BOOT),
    COMBO(combo_kb_reboot, QK_RBT),
    COMBO(combo_sleep, KC_SYSTEM_SLEEP),
    COMBO(combo_enter, KC_ENT),
    COMBO(combo_fr, TO(L_FR)),
    COMBO(combo_se, TO(L_SE)),
    COMBO(combo_en, TO(L_EN)),
    COMBO(combo_base, TO(L_BASE)),
};

// The backspace combo lands on the ~/ keys that I often type in rapid succession, leading to strange behaviour if the
// combo is active. Since I usually type ~/ just after switching to the sym layer, we are here delaying the activation
// of the combo by a half second.

layer_state_t layer_state_set_user(layer_state_t state) {
    // If the symbol layer is turned on, record the current time
    if (get_highest_layer(state) == L_NUMSYM) {
        sym_layer_timer = timer_read();
    } else {
        // Reset the timer when leaving the layer
        sym_layer_timer = 0;
    }
    return state;
}

void housekeeping_task_user(void) {
    static uint32_t glow_timer = 0;
    static uint16_t phase = 0;
    static layer_state_t previous_layer_state = ~(layer_state_t)0;

    bool layer_changed = (layer_state != previous_layer_state);
    bool glow_tick = timer_elapsed32(glow_timer) >= GLOW_INTERVAL_MS;

    if (!layer_changed && !glow_tick) {
        return;
    }

    previous_layer_state = layer_state;

    if (glow_tick) {
        glow_timer = timer_read32();
        phase += GLOW_PHASE_STEP;
    }

    bool left = is_keyboard_left();
    uint8_t start = left ? 0 : 7;

    hsv_t base = persistent_colour(left);

    for (uint8_t i = 0; i < 6; i++) {
        // Mirror the animation across the split.
        uint8_t position = left ? i : (5 - i);

        // 45 degrees between neighbouring LEDs.
        uint16_t led_phase =
            phase + (uint16_t)position * 8192u;

        bool coder =
            !layer_state_cmp(layer_state, L_EN) &&
            !layer_state_cmp(layer_state, L_SE) &&
            !layer_state_cmp(layer_state, L_FR);

        uint8_t value =
            glow_value(base.v, led_phase, coder);

        rgblight_sethsv_at(
            base.h,
            base.s,
            value,
            start + i
        );
    }

    // Top LED remains steady: no animation or dithering.
    hsv_t top = transient_colour();

    rgblight_sethsv_at(
        top.h,
        top.s,
        top.v,
        start + 6
    );
}

// A layer is never toggled in the middle of typing, so the layer combos only
// fire when the keyboard has been idle for a moment. Rolls such as "se" or "en"
// in the middle of a word are thus never mistaken for a layer switch.
#define LAYER_COMBO_IDLE_TERM 250

bool combo_should_trigger(uint16_t combo_index, combo_t *combo, uint16_t keycode, keyrecord_t *record) {
    switch (combo_index) {
        case CMB_FR:
        case CMB_SE:
        case CMB_EN:
        case CMB_BASE:
            if (last_keycode_time != 0 && TIMER_DIFF_16(record->event.time, last_keycode_time) < LAYER_COMBO_IDLE_TERM) {
                return false;
            }
            break;
        default:
            break;
    }
    if (combo->keys == combo_backspace_sym) {
        if (IS_LAYER_ON(L_NUMSYM)) {
            // If the timer is active AND less than 500ms has passed, disable the combo
            if (sym_layer_timer != 0 && timer_elapsed(sym_layer_timer) < 500) {
                return false;
            }
        }
    }
    return true;
}

