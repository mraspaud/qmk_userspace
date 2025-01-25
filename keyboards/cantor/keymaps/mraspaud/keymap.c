// Copyright 2025 Martin Raspaud (@mraspaud)
// SPDX-License-Identifier: GPL-2.0

#include QMK_KEYBOARD_H
#include "keymap_extras/keymap_us_international_linux.h"

// Layers declarations
enum {
    L_BASE = 0,
    L_EN,
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
#define U_HYPHEN UP(HYPHEN, NDASH)
#define U_MINUS UP(MINUS, MDASH)
#define U_ELL  UM(ELL)
#define KC_PSMS S(US_MICR)
#define MAGICFR OSL(L_FRSYM)

enum custom_keycodes {
    DI_QU = SAFE_RANGE,
    CKC_OU,
    DI_TH,
};

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
        case DI_QU:
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
    if (!process_custom_shift_keys(keycode, record)) {
        return false;
    }

    switch (keycode) {
        case DI_QU:
            if (record->event.pressed) {
                if (is_caps_word_on()) {
                    SEND_STRING("QU");
                } else if (is_shift_pressed()) {
                    send_unshifted_string("qu");
                } else {
                    SEND_STRING("qu");
                }
            } else {
                // key release
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
    }
    // your code here
    return true;
}


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
#define CK_NBSP RALT(KC_SPC)
#define CK_NNBS S(RALT(KC_SPC))
#define MT_U LALT_T(KC_U)
#define MT_E RSFT_T(KC_E)
#define MT_N LSFT_T(KC_N)
#define MT_A RCTL_T(KC_A)
#define MT_T LCTL_T(KC_T)

 const custom_shift_key_t custom_shift_keys[] = {
  {KC_DOT , KC_EXLM}, // Shift . is !
  {KC_COMM, KC_COLN}, // Shift , is :
  {US_DQUO, US_GRV }, // Shift " is `
  {KC_UNDS, KC_MINS }, // Shift _ is -
  {KC_LPRN, KC_LBRC }, // Shift ( is [
  {KC_RPRN, KC_RBRC }, // Shift ) is ]
  {KC_LCBR, KC_LABK }, // Shift { is <
  {KC_RCBR, KC_RABK }, // Shift } is >
  {KC_SLSH, US_ASTR }, // Shift / is *
  {US_QUOT, US_RSQU }, // Shift ' is ’
  {US_AT, KC_QUES}, // Shift ! is ?
  {KC_EQL, US_HASH}, // Shift = is #
  {US_RSQU, US_RDQU}, // Shift ‘ is “
  {US_LSQU, US_LDQU}, // Shift ’ is ”
  {US_RDAQ, US_RDQU}, // Shift « is “
  {US_LDAQ, US_LDQU}, // Shift » is ”
  {CK_NNBS, CK_NBSP},
};
uint8_t NUM_CUSTOM_SHIFT_KEYS =
    sizeof(custom_shift_keys) / sizeof(custom_shift_key_t);

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
     /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ ' │ B │ H │ G │ " │       │ @ │ . │ / │ J │ X │ Q │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ C │ S │ N │ T │ K │       │ , │ A │ E │ I │ M │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ [ │ P │ F │ L │ D │ V │       │ = │ U │ O │ Y │ W │ ] │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │Th ├───┐           ┌───┤Qu │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ _ ├───┘
      *                       └───┘   └───┘
      */
    [L_BASE] = LAYOUT_split_3x6_3(
        KC_Z,    US_QUOT, KC_B,    KC_H,    KC_G,    US_DQUO,                                      US_AT,   KC_DOT,  KC_SLSH, KC_J,    KC_X,    KC_Q,
        KC_LPRN, KC_C,    KC_S,    MT_N,    MT_T,    KC_K,                                         KC_COMM, MT_A,    MT_E,    KC_I,    KC_M,    KC_RPRN,
        KC_LCBR, KC_P,    KC_F,    KC_L,    KC_D,    KC_V,                                         KC_EQL,  MT_U,    KC_O,    KC_Y,    KC_W,    KC_RCBR,
                                            DI_QU,   LT_R,  MT_ESC,                       KC_UNDS, LT_SPC,  DI_TH
    ),
     /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ ’ │ B │ H │ G │ " │       │ @ │ . │ / │ J │ X │ Q │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ C │ S │ N │ T │ K │       │ , │ A │ E │ I │ M │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ “ │ P │ F │ L │ D │ V │       │ = │ U │ O │ Y │ W │ ” │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │Th ├───┐           ┌───┤Qu │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ _ ├───┘
      *                       └───┘   └───┘
      */
    [L_EN] = LAYOUT_split_3x6_3(
        _______, US_RSQU, _______, _______, _______, _______,                                      _______, _______, _______, _______, _______,  _______,
        _______, _______, _______, _______, _______, _______,                                      _______, _______, _______, _______, _______,  _______,
        US_LSQU, _______, _______, _______, _______, _______,                                      _______, _______, _______, _______, _______,  US_RSQU,
                                            _______, _______, _______,                   U_HYPHEN, _______, _______
    ),
      /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ ’ │ B │ H │ G │ ” │       │ @ │ . │ Ä │ J │ X │ Q │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ C │ S │ N │ T │ K │       │ , │ A │ E │ I │ M │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ [ │ P │ F │ L │ D │ V │       │ Å │ U │ O │ Y │ W │ ] │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │Th ├───┐           ┌───┤ Ö │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ - ├───┘
      *                       └───┘   └───┘
      */
    [L_SE] = LAYOUT_split_3x6_3(
        _______, US_RSQU, _______, _______, _______, _______,                                      _______, _______, SE_ADIA, _______, _______,  _______,
        _______, _______, _______, _______, _______, _______,                                      _______, _______, _______, _______, _______,  _______,
        US_RDQU, _______, _______, _______, _______, _______,                                      US_ARNG, _______, _______, _______, _______,  US_RDQU,
                                            _______, _______, _______,                   U_HYPHEN, _______, SE_ODIA
    ),
      /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ Ç │ B │ H │ G │ " │       │ @ │ . │ É │ J │ X │ Q │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ C │ S │ N │ T │Mgk│       │ , │ A │ E │ I │ M │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ « │ P │ F │ L │ D │ V │       │ À │ U │ O │ Y │ W │ » │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │Th ├───┐           ┌───┤Qu │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ - ├───┘
      *                       └───┘   └───┘
      */
    [L_FR] = LAYOUT_split_3x6_3(
        _______, US_CCED, _______, _______, _______, _______,                                      _______, _______, US_EACU, _______, _______, _______,
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
      *               │Th ├───┐           ┌───┤Lck│
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ _ ├───┘
      *                       └───┘   └───┘
      */
    [L_NUMSYM] = LAYOUT_split_3x6_3(
        KC_BSLS, US_QUOT, KC_PIPE, KC_AMPR, KC_PERC, _______,                                      KC_PSMS, _______, KC_SLSH, US_TILD, KC_EXLM, US_DEG,
        _______, KC_6,    KC_4,    KC_0,    KC_2,    U_MINUS,                                      _______, KC_3,    KC_1,    KC_5,    KC_7,    _______,
        _______, US_EURO, US_CIRC, KC_DLR,  KC_8,    KC_PLUS,                                      KC_EQL,  KC_9,    US_SCLN, U_ELL,   KC_QUES, _______,
                                            _______, _______, _______,                    KC_UNDS, CK_NNBS, QK_LLCK
    ),
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
        _______, _______, _______, US_ACUT, _______, US_DIAE,                                      _______, RALT(KC_X), U_EGRV,  _______, _______, _______,
        US_LDAQ, US_CEDL, US_DTIL, US_DGRV, US_DCIR, KC_K,                                         _______, U_ACRC,  U_ECRC,  U_ICRC,  US_MICR, US_RDAQ,
        _______, _______, _______, _______, _______, _______,                                      _______, U_UCRC,  U_OCRC,  CKC_OU,  US_OE, _______,
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
};


const uint16_t PROGMEM combo_tab[] = {KC_B, KC_H, COMBO_END};
const uint16_t PROGMEM combo_stab[] = {US_RSQU, KC_B, COMBO_END};
const uint16_t PROGMEM combo_backspace[] = {KC_SLASH, KC_J, COMBO_END};
const uint16_t PROGMEM combo_backspace_se[] = {SE_ADIA, KC_J, COMBO_END};
const uint16_t PROGMEM combo_backspace_fr[] = {US_EACU, KC_J, COMBO_END};
const uint16_t PROGMEM combo_backspace_sym[] = {KC_SLSH, US_TILD, COMBO_END};
const uint16_t PROGMEM combo_delete[] = { KC_J, KC_X, COMBO_END};
const uint16_t PROGMEM combo_prtscr[] = {US_AT, KC_DOT, COMBO_END};
const uint16_t PROGMEM combo_capsword[] = {KC_LPRN, KC_RPRN, COMBO_END};
const uint16_t PROGMEM combo_bootloader[] = {US_DQUO, US_AT, COMBO_END};
const uint16_t PROGMEM combo_sleep[] = {KC_COMM, KC_EQL, COMBO_END};
const uint16_t PROGMEM combo_enter[] = {MT_A, KC_I, COMBO_END};
const uint16_t PROGMEM combo_q[] = {KC_Y, KC_W, COMBO_END};
const uint16_t PROGMEM combo_q_se[] = {US_ODIA, KC_W, COMBO_END};
const uint16_t PROGMEM combo_en[] = {MT_N, MT_E, COMBO_END};
const uint16_t PROGMEM combo_se[] = {KC_S, MT_E, COMBO_END};
const uint16_t PROGMEM combo_fr[] = {KC_F, LT_R, COMBO_END};
const uint16_t PROGMEM combo_fn[] = {LT_SPC, LT_R, COMBO_END};
const uint16_t PROGMEM combo_base[] = {KC_F, KC_L, KC_D, COMBO_END};
combo_t key_combos[] = {
    COMBO(combo_tab, KC_TAB),
    COMBO(combo_stab, S(KC_TAB)),
    COMBO(combo_backspace, KC_BSPC),
    COMBO(combo_backspace_se, KC_BSPC),
    COMBO(combo_backspace_fr, KC_BSPC),
    COMBO(combo_backspace_sym, KC_BSPC),
    COMBO(combo_delete, KC_DELETE),
    COMBO(combo_prtscr, KC_PRINT_SCREEN),
    COMBO(combo_capsword, CW_TOGG),
    COMBO(combo_bootloader, QK_BOOT),
    COMBO(combo_sleep, KC_SYSTEM_SLEEP),
    COMBO(combo_enter, KC_ENT),
    COMBO(combo_q, KC_Q),
    COMBO(combo_q_se, KC_Q),
    COMBO(combo_fr, TO(L_FR)),
    COMBO(combo_se, TO(L_SE)),
    COMBO(combo_en, TO(L_EN)),
    COMBO(combo_base, TO(L_BASE)),
    COMBO(combo_fn, OSL(L_FN)),
};
