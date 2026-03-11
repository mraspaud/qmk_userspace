// Copyright 2025 Martin Raspaud (@mraspaud)
// SPDX-License-Identifier: GPL-2.0
#include QMK_KEYBOARD_H
#include "keymap_extras/keymap_eurkey.h"
#define QU_TIMEOUT 1000
static uint16_t q_timer = 0;
static uint16_t last_keycode = 0;
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
#define MT_LPRN LCTL_T(KC_LPRN)
#define MT_RPRN RCTL_T(KC_RPRN)
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
        case LCTL_T(KC_LPRN):
            if (record->tap.count && record->event.pressed) {
                tap_code16(KC_LPRN); // Send KC_DQUO on tap
                return false;        // Return false to ignore further processing of key
            }
            break;
        case RCTL_T(KC_RPRN):
            if (record->tap.count && record->event.pressed) {
                tap_code16(KC_RPRN); // Send KC_DQUO on tap
                return false;        // Return false to ignore further processing of key
            }
            break;
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
                if (last_keycode != EU_COLN) {
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
        last_keycode = keycode;
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


 const custom_shift_key_t custom_shift_keys[] = {
  {KC_DOT , KC_EXLM}, // Shift . is !
  {KC_COMM, KC_QUES}, // Shift , is :
  {KC_COLN, KC_SCLN}, // Shift : is ;
  {EU_DQUO, EU_GRV }, // Shift " is `
  {KC_UNDS, KC_MINS }, // Shift _ is -
  {KC_LBRC, KC_LCBR }, // Shift [ is {
  {KC_RBRC, KC_RCBR }, // Shift ] is }
  {MT_LPRN, KC_LABK }, // Shift ( is <
  {MT_RPRN, KC_RABK }, // Shift } is >
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
        MT_LPRN, MT_S,    KC_C,    MT_N,    KC_T,    KC_K,                                         KC_COMM, KC_A,    MT_E,    KC_I,    MT_M,    MT_RPRN,
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
      * │ [ │ ^ │ € │ § │ 8 │ $ │       │ = │ 9 │ … │ · │ ? │ ] │
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
        KC_LBRC, EU_CIRC, SN_CURR, EU_SECT, KC_8,    KC_DLR,                                       _______, KC_9,    EU_ELLP, EU_MDDT, EU_THRN, KC_RBRC,
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
        _______, KC_LGUI, OS_LALT, OS_LSFT, OS_LCTL, _______,                                      KC_HOME, KC_LEFT, KC_DOWN, KC_RGHT, KC_END,  KC_VOLD,
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


const uint16_t PROGMEM combo_tab[] = {KC_W, KC_H, COMBO_END};
const uint16_t PROGMEM combo_tab_fr[] = {EU_CCED, KC_H, COMBO_END};
const uint16_t PROGMEM combo_btab[] = {KC_W, KC_B, COMBO_END};
const uint16_t PROGMEM combo_backspace[] = {KC_SLASH, KC_J, COMBO_END};
const uint16_t PROGMEM combo_delete[] = {KC_X, KC_J, COMBO_END};
const uint16_t PROGMEM combo_backspace_se[] = {SE_ADIA, KC_J, COMBO_END};
const uint16_t PROGMEM combo_backspace_fr[] = {EU_EACU, KC_J, COMBO_END};
const uint16_t PROGMEM combo_backspace_sym[] = {KC_SLSH, CKC_TLD, COMBO_END};
const uint16_t PROGMEM combo_prtscr[] = {EU_COLN, KC_DOT, COMBO_END};
const uint16_t PROGMEM combo_capsword[] = {MT_LPRN, MT_RPRN, COMBO_END};
const uint16_t PROGMEM combo_bootloader[] = {EU_DQUO, EU_COLN, COMBO_END};
const uint16_t PROGMEM combo_kb_reboot[] = {EU_AT, KC_Z, COMBO_END};
const uint16_t PROGMEM combo_sleep[] = {KC_COMM, KC_EQL, COMBO_END};
const uint16_t PROGMEM combo_enter[] = {KC_A, KC_I, COMBO_END};
const uint16_t PROGMEM combo_en[] = {MT_N, MT_E, COMBO_END};
const uint16_t PROGMEM combo_se[] = {MT_S, MT_E, COMBO_END};
const uint16_t PROGMEM combo_fr[] = {KC_F, LT_R, COMBO_END};
const uint16_t PROGMEM combo_base[] = {KC_P, KC_L, KC_D, COMBO_END};
combo_t key_combos[] = {
    COMBO(combo_tab, KC_TAB),
    COMBO(combo_tab_fr, KC_TAB),
    COMBO(combo_btab, RSFT(KC_TAB)),
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

bool combo_should_trigger(uint16_t combo_index, combo_t *combo, uint16_t keycode, keyrecord_t *record) {
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

