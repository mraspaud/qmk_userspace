// Copyright 2022 Diego Palacios (@diepala)
// SPDX-License-Identifier: GPL-2.0

#include QMK_KEYBOARD_H
#include "keymap_extras/keymap_us_international_linux.h"

// Layers declarations
enum {
    L_BASE = 0,
    L_SE,
    L_NUMSYM,
    L_NAV,
    L_SYM,
    L_FR,
    L_FRSYM,
    L_FN,
};

enum unicode_names {
    ELL,
    HSH,
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
};

const uint32_t PROGMEM unicode_map[] = {
    [ELL]  = 0x2026,  // …
    [HSH] = 0x0023,  // #
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
};

#define U_AGRV UP(AGRV, AAGRV)
#define U_ACRC UP(ACRC, AACRC)
#define U_EGRV UP(EGRV, EEGRV)
#define U_ECRC UP(ECRC, EECRC)
#define U_ICRC UP(ICRC, IICRC)
#define U_OCRC UP(OCRC, OOCRC)
#define U_UGRV UP(UGRV, UUGRV)
#define U_UCRC UP(UCRC, UUCRC)
#define U_ELL  UM(ELL)
#define KC_PSMS S(US_MICR)
#define MAGICFR OSL(L_FRSYM)

enum custom_keycodes {
    SMTD_KEYCODES_BEGIN = SAFE_RANGE,
    CKC_N,
    CKC_T,
    CKC_A,
    CKC_E,
    CKC_SPC,
    CKC_R,
    CKC_UND,
    SMTD_KEYCODES_END,
    CKC_QU,
    CKC_OU,
};

#include "sm_td.h"
#include "features/custom_shift_keys.h"


bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_smtd(keycode, record)) {
        return false;
    }
    if (!process_custom_shift_keys(keycode, record)) {
        return false;
    }
    bool shift_pressed = false;

    switch (keycode) {
        case KC_LSFT:
        case KC_RSFT:
            shift_pressed = true;
            break;
        case CKC_QU:
            if (record->event.pressed) {
                // when keycode QMKBEST is pressed
                SEND_STRING("qu");
            } else {
                // when keycode QMKBEST is released
            }
            break;
        case CKC_OU:
            if (record->event.pressed) {
                if (shift_pressed) {
                    SEND_STRING("O");
                } else {
                    SEND_STRING("o");
                }
                send_unicode_string("ù");
            } else {
                // when keycode QMKBEST is released
            }
            break;
    }
    // your code here
    return true;
}


void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch (keycode) {
        SMTD_MT(CKC_N, KC_N, KC_LEFT_SHIFT)
        SMTD_MT(CKC_T, KC_T, KC_LEFT_CTRL)
        SMTD_MT(CKC_A, KC_A, KC_RIGHT_CTRL)
        SMTD_MT(CKC_E, KC_E, KC_RIGHT_SHIFT)
        SMTD_LT(CKC_SPC, KC_SPACE, L_NAV)
        SMTD_LT(CKC_R, KC_R, L_NUMSYM)
    }
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

 const custom_shift_key_t custom_shift_keys[] = {
  {KC_DOT , KC_EXLM}, // Shift . is !
  {KC_COMM, KC_COLN}, // Shift , is :
  {US_DQUO, US_GRV }, // Shift " is `
  {KC_UNDS, KC_MINS }, // Shift _ is -
  {KC_LPRN, KC_LCBR }, // Shift ( is {
  {KC_RPRN, KC_RCBR }, // Shift ) is }
  {KC_LBRC, KC_LABK }, // Shift [ is <
  {KC_RBRC, KC_RABK }, // Shift ] is >
  {KC_SLSH, US_ASTR }, // Shift / is *
  {US_RSQU, US_QUOT }, // Shift ’ is '
  {US_AT, KC_QUES}, // Shift ! is ?
  {KC_EQL, US_HASH}, // Shift = is #
};
uint8_t NUM_CUSTOM_SHIFT_KEYS =
    sizeof(custom_shift_keys) / sizeof(custom_shift_key_t);

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
     /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ ’ │ B │ H │ G │ " │       │ @ │ . │ / │ J │ X │ Q │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ C │ S │ N │ T │ K │       │ , │ A │ E │ I │ M │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ [ │ P │ F │ L │ D │ V │       │ = │ U │ O │ Y │ W │ ] │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │ Z ├───┐           ┌───┤ Q │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ _ ├───┘
      *                       └───┘   └───┘
      */
    [L_BASE] = LAYOUT_split_3x6_3(
        KC_Z,    US_RSQU, KC_B,    KC_H,    KC_G,    US_DQUO,                                      US_AT,   KC_DOT,  KC_SLSH, KC_J,    KC_X,    CKC_QU,
        KC_LPRN, KC_C,    KC_S,    CKC_N,   CKC_T,   KC_K,                                         KC_COMM, CKC_A,   CKC_E,   KC_I,    KC_M,    KC_RPRN,
        KC_LBRC, KC_P,    KC_F,    KC_L,    KC_D,    KC_V,                                         KC_EQL,  KC_U,    KC_O,    KC_Y,    KC_W,    KC_RBRC,
                                            KC_Z,    CKC_R, KC_ESC,                       KC_UNDS, CKC_SPC, KC_Q
    ),
      /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ ’ │ B │ H │ G │ " │       │ Å │ . │ Ä │ J │ X │ Q │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ C │ S │ N │ T │ K │       │ , │ A │ E │ I │ M │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ [ │ P │ F │ L │ D │ V │       │ Ö │ U │ O │ Y │ W │ ] │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │ Z ├───┐           ┌───┤ Q │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ _ ├───┘
      *                       └───┘   └───┘
      */
    [L_SE] = LAYOUT_split_3x6_3(
        _______, _______, _______, _______, _______, _______,                                      US_ARNG, _______, SE_ADIA, _______, _______,  KC_Q,
        _______, _______, _______, _______, _______, _______,                                      _______, _______, _______, _______, _______,  _______,
        _______, _______, _______, _______, _______, _______,                                      SE_ODIA, _______, _______, _______, _______,  _______,
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
      *               │ Z ├───┐           ┌───┤ Q │
      *               └───┤Alt├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ _ ├───┘
      *                       └───┘   └───┘
      */
    [L_NAV] = LAYOUT_split_3x6_3(
        _______, _______, _______, _______, _______, _______,                                      KC_BRID, KC_PGDN, KC_UP,   KC_PGUP, KC_BRIU, KC_VOLU,
        _______, KC_LGUI, KC_LALT, KC_LSFT, KC_LCTL, _______,                                      KC_HOME, KC_LEFT, KC_DOWN, KC_RGHT, KC_END,  KC_VOLD,
        _______, _______, _______, _______, _______, _______,                                      _______, KC_MPRV, KC_MPLY, KC_MNXT, KC_MSTP, KC_MUTE,
                                            _______, KC_LALT, _______,                    _______, _______, _______
    ),
        /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ ’ │ B │ H │ G │ " │       │ @ │ . │ / │ J │ X │ Q │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ C │ S │ N │ T │ K │       │ , │ A │ E │ I │ M │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ [ │ P │ F │ L │ D │ V │       │ = │ U │ O │ Y │ W │ ] │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │ Z ├───┐           ┌───┤ Q │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ _ ├───┘
      *                       └───┘   └───┘
      */
    [L_NUMSYM] = LAYOUT_split_3x6_3(
        KC_BSLS, US_QUOT, KC_PIPE, KC_AMPR, KC_PERC, _______,                                      _______, _______, _______, US_TILD, US_SCLN, US_DEG,
        _______, KC_6,    KC_4,    KC_0,    KC_2,    KC_MINS,                                      _______, KC_3,    KC_1,    KC_5,    KC_7,    _______,
        _______, US_CIRC, US_EURO, KC_DLR,  KC_8,    KC_PLUS,                                      _______, KC_9,    KC_EXLM, U_ELL,   KC_QUES, _______,
                                            _______, _______, _______,                    _______, QK_LLCK, _______
    ),
      /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ " │ B │ H │ G │ ’ │       │ @ │ . │ É │ J │ X │ Q │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ C │ S │ N │ T │Mgk│       │ , │ A │ E │ I │ M │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ [ │ P │ F │ L │ D │ V │       │ À │ U │ O │ Y │ W │ ] │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │ Z ├───┐           ┌───┤ Q │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤Esc│   │ - ├───┘
      *                       └───┘   └───┘
      */
    [L_FR] = LAYOUT_split_3x6_3(
        _______, US_DQUO, _______, _______, _______, US_RSQU,                                      _______, _______, US_EACU, _______, _______, _______,
        _______, _______, _______, _______, _______, MAGICFR,                                      _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______,                                      U_AGRV,  _______, _______, _______, _______, _______,
                                            _______, _______, _______,                    KC_MINS, _______, _______
    ),
      /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │ Z │ " │ B │ ' │ G │ " │       │ @ │ . │ È │ J │ X │ Q │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ « │ Ç │ ~ │ ` │ ^ │ K │       │ , │ Ê │ Ê │ Î │ µ │ » │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ [ │ P │ F │ L │ D │ V │       │ À │ Û │ Ô │Où │ Œ │ ] │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │ Z ├───┐           ┌───┤ Q │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤ _ │   │Esc├───┘
      *                       └───┘   └───┘
      */
    [L_FRSYM] = LAYOUT_split_3x6_3(
        _______, _______, _______, US_ACUT, _______, US_DIAE,                                      _______, _______, U_EGRV,  _______, _______, _______,
        US_LDAQ, US_CCED, US_DTIL, US_DGRV, US_DCIR, KC_K,                                         _______, U_ACRC,  U_ECRC,  U_ICRC,  US_MICR, US_RDAQ,
        _______, _______, _______, _______, _______, _______,                                      _______, U_UCRC,  U_OCRC,  CKC_OU,  US_OE, _______,
                                            _______, _______, _______,                 RALT(KC_X), U_ELL,   _______
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
      *               │ Z ├───┐           ┌───┤ Q │
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


const uint16_t PROGMEM combo_tab[] = {KC_B, KC_H, COMBO_END};
const uint16_t PROGMEM combo_stab[] = {US_RSQU, KC_B, COMBO_END};
const uint16_t PROGMEM combo_backspace[] = {KC_SLASH, KC_J, COMBO_END};
const uint16_t PROGMEM combo_backspace_se[] = {SE_ADIA, KC_J, COMBO_END};
const uint16_t PROGMEM combo_backspace_fr[] = {US_EACU, KC_J, COMBO_END};
const uint16_t PROGMEM combo_delete[] = { KC_J, KC_X, COMBO_END};
const uint16_t PROGMEM combo_prtscr[] = {US_AT, KC_DOT, COMBO_END};
const uint16_t PROGMEM combo_capsword[] = {KC_LPRN, KC_RPRN, COMBO_END};
const uint16_t PROGMEM combo_bootloader[] = {US_DQUO, US_AT, COMBO_END};
const uint16_t PROGMEM combo_sleep[] = {KC_COMM, KC_EQL, COMBO_END};
const uint16_t PROGMEM combo_enter[] = {CKC_A, KC_I, COMBO_END};
const uint16_t PROGMEM combo_q[] = {KC_Y, KC_W, COMBO_END};
const uint16_t PROGMEM combo_q_se[] = {US_ODIA, KC_W, COMBO_END};
const uint16_t PROGMEM combo_se[] = {KC_S, CKC_E, COMBO_END};
const uint16_t PROGMEM combo_fr[] = {KC_F, CKC_R, COMBO_END};
const uint16_t PROGMEM combo_fn[] = {CKC_SPC, CKC_R, COMBO_END};
combo_t key_combos[] = {
    COMBO(combo_tab, KC_TAB),
    COMBO(combo_stab, S(KC_TAB)),
    COMBO(combo_backspace, KC_BSPC),
    COMBO(combo_backspace_se, KC_BSPC),
    COMBO(combo_backspace_fr, KC_BSPC),
    COMBO(combo_delete, KC_DELETE),
    COMBO(combo_prtscr, KC_PRINT_SCREEN),
    COMBO(combo_capsword, CW_TOGG),
    COMBO(combo_bootloader, QK_BOOT),
    COMBO(combo_sleep, KC_SYSTEM_SLEEP),
    COMBO(combo_enter, KC_ENT),
    COMBO(combo_q, KC_Q),
    COMBO(combo_q_se, KC_Q),
    COMBO(combo_se, TG(L_SE)),
    COMBO(combo_fr, TG(L_FR)),
    COMBO(combo_fn, OSL(L_FN)),
};
