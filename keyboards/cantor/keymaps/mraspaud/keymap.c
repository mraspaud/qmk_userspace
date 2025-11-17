// Copyright 2025 Martin Raspaud (@mraspaud)
// SPDX-License-Identifier: GPL-2.0
#include QMK_KEYBOARD_H
#include "keymap_extras/keymap_us_international_linux.h"
#define QU_TIMEOUT 500
static uint16_t q_timer = 0;
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
    ELL,
    AGRV,
    AAGRV,
    ACRC,
    AACRC,
    EGRV,
    EEGRV,
    ECRC,
    EECRC,
    ICRC,
    IICRC,
    OCRC,
    OOCRC,
    UGRV,
    UUGRV,
    UCRC,
    UUCRC,
    NDASH,
    MDASH,
    MINUS,
    HYPHEN,
};

const uint32_t PROGMEM unicode_map[] = {
    [ELL]  = 0x2026,  // …
    [AGRV] = 0x00E0,  // à
    [AAGRV] = 0x00C0,  // À
    [ACRC] = 0x00E2,  // â
    [AACRC] = 0x00C2,  // Â
    [EGRV] = 0x00E8,  // è
    [EEGRV] = 0x00C8,  // È
    [ECRC] = 0x00EA,  // ê
    [EECRC] = 0x00CA,  // Ê
    [ICRC] = 0x00EE,  // î
    [IICRC] = 0x00CE,  // Î
    [OCRC] = 0x00F4,  // ô
    [OOCRC] = 0x00D4,  // Ô
    [UGRV] = 0x00F9,  // ù
    [UUGRV] = 0x00D9,  // Ù
    [UCRC] = 0x00FB,  // û
    [UUCRC] = 0x00DB,  // Û
    [NDASH] = 0x2013,  // –
    [MDASH] = 0x2014,  // —
    [MINUS] = 0x2212,  // −
    [HYPHEN] = 0x002D,  // -
};

#define U_AGRV UP(AGRV, AAGRV)
#define U_ACRC UP(ACRC, AACRC)
#define U_EGRV UP(EGRV, EEGRV)
#define U_ECRC UP(ECRC, EECRC)
#define U_ICRC UP(ICRC, IICRC)
#define U_OCRC UP(OCRC, OOCRC)
#define U_UGRV UP(UGRV, UUGRV)
#define U_UCRC UP(UCRC, UUCRC)
#define U_DASH UP(NDASH, MDASH)
#define U_HYPHEN UP(NDASH, HYPHEN)
#define U_MINUS UP(MINUS, MDASH)
#define U_ELL  UM(ELL)
#define KC_PSMS S(US_MICR)
#define MAGICFR OSL(L_FRSYM)

enum custom_keycodes {
    CKC_OU = SAFE_RANGE,
    DI_TH,
};

// Tap Dance declarations
enum {
    TD_ADIA,
    TD_ODIA,
};

// Tap Dance definitions
tap_dance_action_t tap_dance_actions[] = {
    // Tap once for Escape, twice for Caps Lock
    [TD_ADIA] = ACTION_TAP_DANCE_DOUBLE(US_ADIA, US_AE),
    [TD_ODIA] = ACTION_TAP_DANCE_DOUBLE(US_ODIA, US_OSTR),
};

#define SE_ODIA TD(TD_ODIA)
#define SE_ADIA TD(TD_ADIA)
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
#define MT_DGRV LSFT_T(US_DGRV)

#include "features/custom_shift_keys.h"

bool is_shift_pressed(void) {
    const uint8_t saved_mods = get_mods();
#ifndef NO_ACTION_ONESHOT
    const uint8_t mods = saved_mods | get_weak_mods() | get_oneshot_mods();
#else
    const uint8_t mods = saved_mods | get_weak_mods();
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
    if (!process_custom_shift_keys(keycode, record)) {
        return false;
    }

    switch (keycode) {
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
                if (is_shift_pressed()) {
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
                } else if (is_shift_pressed()) {
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
                q_timer = timer_read();
            }
            break;
        case KC_A:
        case MT_E:
        case KC_I:
        case KC_O:
        case KC_Y:
        case US_EACU:
        case U_ACRC:
        case U_ECRC:
        case U_EGRV:
        case U_ICRC:
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
            q_timer = 0;
            break;
    }
    // your code here
    return true;
}


 const custom_shift_key_t custom_shift_keys[] = {
  {KC_DOT , KC_EXLM}, // Shift . is !
  {KC_COMM, KC_COLN}, // Shift , is :
  {US_DQUO, US_GRV }, // Shift " is `
  {KC_UNDS, KC_MINS }, // Shift _ is -
  {KC_LBRC, KC_LCBR }, // Shift [ is {
  {KC_RBRC, KC_RCBR }, // Shift ] is }
  {MT_LPRN, KC_LABK }, // Shift ( is <
  {MT_RPRN, KC_RABK }, // Shift } is >
  {KC_SLSH, US_ASTR }, // Shift / is *
  {US_QUOT, US_RSQU }, // Shift ' is ’
  {US_AT, KC_QUES}, // Shift ! is ?
  {KC_EQL, US_HASH}, // Shift = is #
  {US_RSQU, US_RDQU}, // Shift ‘ is “
  {US_LSQU, US_LDQU}, // Shift ’ is ”
  {US_RDAQ, US_RDQU}, // Shift « is “
  {US_LDAQ, US_LDQU}, // Shift » is ”
  {US_DEG, US_IQUE}, // Shift ° is ¿
  {CK_NNBS, CK_NBSP},
  {KC_BSPC, KC_DELETE},
};
uint8_t NUM_CUSTOM_SHIFT_KEYS =
    sizeof(custom_shift_keys) / sizeof(custom_shift_key_t);

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
     /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ B │ W │ H │ G │ " │       │ * │ . │ / │ J │ X │ @ │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ S │ C │ N │ T │ K │       │ , │ A │ E │ I │ M │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ [ │ F │ P │ L │ D │ V │       │ = │ U │ O │ Y │ þ │ ] │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │ Q ├───┐           ┌───┤ ' │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ _ ├───┘
      *                       └───┘   └───┘
      */
    [L_BASE] = LAYOUT_split_3x6_3(
        KC_Z,    KC_B,    KC_W,    KC_H,    KC_G,    US_DQUO,                                      US_ASTR, KC_DOT,  KC_SLSH, KC_J,    KC_X,    US_AT,
        MT_LPRN, KC_S,    KC_C,    MT_N,    KC_T,    KC_K,                                         KC_COMM, KC_A,    MT_E,    KC_I,    KC_M,    MT_RPRN,
        KC_LBRC, KC_F,    KC_P,    KC_L,    KC_D,    KC_V,                                         KC_EQL,  KC_U,    KC_O,    KC_Y,    DI_TH,   KC_RBRC,
                                            KC_Q,    LT_R,  MT_ESC,                       KC_UNDS, LT_SPC,  US_QUOT
    ),
     /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ B │ W │ H │ G │ " │       │ @ │ . │ / │ J │ X │ Q │
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
        _______, _______, _______, _______, _______, _______,                                      US_RSQU, _______, _______, _______, _______,  _______,
        _______, _______, _______, _______, _______, _______,                                      _______, _______, _______, _______, _______,  _______,
        US_LSQU, _______, _______, _______, _______, _______,                                      _______, _______, _______, _______, _______,  US_RSQU,
                                            _______, _______, _______,                   U_HYPHEN, _______, US_RSQU
    ),
      /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ W │ B │ H │ G │ ” │       │ @ │ . │ Ä │ J │ X │ Q │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ C │ S │ N │ T │ K │       │ , │ A │ E │ I │ M │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ” │ P │ F │ L │ D │ V │       │ Å │ U │ O │ Y │ Ö │ ” │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │Th ├───┐           ┌───┤ Ö │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ - ├───┘
      *                       └───┘   └───┘
      */
    [L_SE] = LAYOUT_split_3x6_3(
        _______, _______, _______, _______, _______, _______,                                      US_RSQU, _______, SE_ADIA, _______, _______,  _______,
        _______, _______, _______, _______, _______, _______,                                      _______, _______, _______, _______, _______,  _______,
        US_RDQU, _______, _______, _______, _______, _______,                                      US_ARNG, _______, _______, _______, SE_ODIA,  US_RDQU,
                                            _______, _______, _______,                   U_HYPHEN, _______, US_RSQU
    ),
      /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ Ç │ B │ H │ G │ K │       │ @ │ . │ É │ J │ X │ Q │
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
        _______, _______, US_CCED, _______, _______, KC_K,                                         US_RSQU, _______, US_EACU, _______, _______, _______,
        _______, _______, _______, _______, _______, MAGICFR,                                      _______, _______, _______, _______, _______, _______,
        US_LDAQ, _______, _______, _______, _______, _______,                                      U_AGRV,  _______, _______, _______, _______, US_RDAQ,
                                            _______, _______, _______,                   U_HYPHEN, _______, US_RSQU
    ),
        /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ \ │ ' │ | │ & │ % │ " │       │ @ │ . │ / │ ~ │ ; │ ° │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ 6 │ 4 │ 0 │ 2 │ - │       │ , │ 3 │ 1 │ 5 │ 7 │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ [ │ ^ │ € │ $ │ 8 │ + │       │ = │ 9 │ ! │ … │ ? │ ] │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │Qu ├───┐           ┌───┤Lck│
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ _ ├───┘
      *                       └───┘   └───┘
      */
    [L_NUMSYM] = LAYOUT_split_3x6_3(
        KC_BSLS, US_QUOT, KC_PIPE, KC_AMPR, KC_PERC, _______,                                      KC_PSMS, _______, KC_SLSH, US_TILD, US_MICR, US_DEG,
        _______, KC_6,    KC_4,    KC_0,    KC_2,    U_MINUS,                                      _______, KC_3,    KC_1,    KC_5,    KC_7,    _______,
        KC_LBRC, US_EURO, US_CIRC, KC_DLR,  KC_8,    KC_PLUS,                                      KC_EQL,  KC_9,    US_SCLN, U_ELL,   US_THRN, KC_RBRC,
                                            _______, _______, _______,                    KC_UNDS, CK_NNBS, QK_LLCK
    ),
       /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ ’ │ B │ H │ G │ " │       │brd│pdn│up │pup│bru│vup│
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │Gui│Alt│Sft│Ctl│ K │       │hom│lft│dn │rgt│end│vdn│
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ [ │ P │ F │ L │ D │ V │       │ = │prv│ply│nxt│stp│mut│
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │Th ├───┐           ┌───┤Qu │
      *               └───┤Alt├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ _ ├───┘
      *                       └───┘   └───┘
      */
    [L_NAV] = LAYOUT_split_3x6_3(
        _______, _______, _______, _______, _______, _______,                                      KC_BRID, KC_PGDN, KC_UP,   KC_PGUP, KC_BRIU, KC_VOLU,
        _______, KC_LGUI, OS_LALT, OS_LSFT, OS_LCTL, _______,                                      KC_HOME, KC_LEFT, KC_DOWN, KC_RGHT, KC_END,  KC_VOLD,
        _______, _______, _______, _______, _______, _______,                                      _______, KC_MPRV, KC_MPLY, KC_MNXT, KC_MSTP, KC_MUTE,
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
        _______, _______, KC_W,    US_ACUT, _______, US_DIAE,                                      _______, RALT(KC_X), U_EGRV,  _______, _______, _______,
        _______, US_CEDL, US_DTIL, MT_DGRV, US_DCIR, KC_K,                                         _______, U_ACRC,  U_ECRC,  U_ICRC,  _______, _______,
        _______, _______, _______, _______, _______, _______,                                      _______, U_UCRC,  U_OCRC,  CKC_OU,  US_OE,   _______,
                                            _______, _______, _______,                    _______, _______,   _______
    ),
      /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ ’ │ B │ H │ G │ " │       │ # │ . │ / │ J │ X │ Q │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ C │ S │ N │ T │ K │       │ , │ A │ E │ I │ M │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ [ │ P │ F │ L │ D │ V │       │ = │ U │ O │ Y │ W │ ] │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │Th ├───┐           ┌───┤Qu │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤ _ │   │Esc├───┘
      *                       └───┘   └───┘
      */
    [L_FN] = LAYOUT_split_3x6_3(
        _______, _______, _______, _______, _______, _______,                                      _______, KC_F9,   KC_F10,  KC_F11,  KC_F12,  _______,
        _______, _______, _______, KC_LSFT, _______, _______,                                      _______, KC_F5,   KC_F6,   KC_F7,   KC_F8,   _______,
        _______, _______, _______, _______, _______, _______,                                      _______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   _______,
                                            _______, _______, _______,                    _______, _______, _______
    ),
};


const uint16_t PROGMEM combo_tab[] = {KC_W, KC_H, COMBO_END};
const uint16_t PROGMEM combo_qu[] = {KC_J, KC_X, COMBO_END};
const uint16_t PROGMEM combo_backspace[] = {KC_SLASH, KC_J, COMBO_END};
const uint16_t PROGMEM combo_backspace_se[] = {SE_ADIA, KC_J, COMBO_END};
const uint16_t PROGMEM combo_backspace_fr[] = {US_EACU, KC_J, COMBO_END};
const uint16_t PROGMEM combo_backspace_sym[] = {KC_SLSH, US_TILD, COMBO_END};
const uint16_t PROGMEM combo_prtscr[] = {US_AT, KC_DOT, COMBO_END};
const uint16_t PROGMEM combo_capsword[] = {MT_LPRN, MT_RPRN, COMBO_END};
const uint16_t PROGMEM combo_bootloader[] = {US_DQUO, US_ASTR, COMBO_END};
const uint16_t PROGMEM combo_sleep[] = {KC_COMM, KC_EQL, COMBO_END};
const uint16_t PROGMEM combo_enter[] = {KC_A, KC_I, COMBO_END};
const uint16_t PROGMEM combo_en[] = {MT_N, MT_E, COMBO_END};
const uint16_t PROGMEM combo_se[] = {KC_S, MT_E, COMBO_END};
const uint16_t PROGMEM combo_fr[] = {KC_F, LT_R, COMBO_END};
const uint16_t PROGMEM combo_fn[] = {LT_SPC, LT_R, COMBO_END};
const uint16_t PROGMEM combo_base[] = {KC_P, KC_L, KC_D, COMBO_END};
combo_t key_combos[] = {
    COMBO(combo_tab, KC_TAB),
    COMBO(combo_qu, KC_Q),
    COMBO(combo_backspace, KC_BSPC),
    COMBO(combo_backspace_se, KC_BSPC),
    COMBO(combo_backspace_fr, KC_BSPC),
    COMBO(combo_backspace_sym, KC_BSPC),
    COMBO(combo_prtscr, KC_PRINT_SCREEN),
    COMBO(combo_capsword, CW_TOGG),
    COMBO(combo_bootloader, QK_BOOT),
    COMBO(combo_sleep, KC_SYSTEM_SLEEP),
    COMBO(combo_enter, KC_ENT),
    COMBO(combo_fr, TO(L_FR)),
    COMBO(combo_se, TO(L_SE)),
    COMBO(combo_en, TO(L_EN)),
    COMBO(combo_base, TO(L_BASE)),
    COMBO(combo_fn, OSL(L_FN)),
};
