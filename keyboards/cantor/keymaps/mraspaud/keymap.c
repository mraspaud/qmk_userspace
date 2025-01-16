// Copyright 2022 Diego Palacios (@diepala)
// SPDX-License-Identifier: GPL-2.0

#include QMK_KEYBOARD_H
#include "keymap_extras/keymap_us_international_linux.h"

// Layers declarations
enum {
    L_BASE = 0,
    L_SE,
    L_NUM,
    L_NAV,
    L_SYM,
    L_FR,
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

enum custom_keycodes {
    SMTD_KEYCODES_BEGIN = SAFE_RANGE,
    CKC_C, // reads as C(ustom) + KC_A, but you may give any name here
    CKC_S,
    CKC_N,
    CKC_T,
    CKC_A,
    CKC_E,
    CKC_I,
    CKC_M,
    CKC_SC,
    CKC_SS,
    CKC_ST,
    CKC_SA,
    CKC_SI,
    CKC_SM,
    CKC_DLR,
    CKC_SPC,
    CKC_R,
    CKC_5,
    CKC_Q,
    CKC_Z,
    CKC_UND,
    SMTD_KEYCODES_END,
    CKC_QU,
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
    switch (keycode) {
    case CKC_QU:
        if (record->event.pressed) {
            // when keycode QMKBEST is pressed
            SEND_STRING("qu");
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
        SMTD_MT(CKC_C, KC_C, KC_LEFT_GUI)
        SMTD_MT(CKC_S, KC_S, KC_LEFT_ALT)
        SMTD_MT(CKC_N, KC_N, KC_LEFT_SHIFT)
        SMTD_MT(CKC_T, KC_T, KC_LEFT_CTRL)
        SMTD_MT(CKC_A, KC_A, KC_RIGHT_CTRL)
        SMTD_MT(CKC_E, KC_E, KC_RIGHT_SHIFT)
        SMTD_MT(CKC_I, KC_I, KC_LEFT_ALT)
        SMTD_MT(CKC_M, KC_M, KC_RIGHT_GUI)
        SMTD_MT(CKC_SC, S(KC_C), KC_LEFT_GUI)
        SMTD_MT(CKC_SS, S(KC_S), KC_LEFT_ALT)
        SMTD_MT(CKC_ST, S(KC_T), KC_LEFT_CTRL)
        SMTD_MT(CKC_SA, S(KC_A), KC_RIGHT_CTRL)
        SMTD_MT(CKC_SI, S(KC_I), KC_LEFT_ALT)
        SMTD_MT(CKC_SM, S(KC_M), KC_RIGHT_GUI)
        SMTD_MT(CKC_DLR, US_DLR, KC_LEFT_SHIFT)
        SMTD_LT(CKC_SPC, KC_SPACE, L_NAV)
        SMTD_LT(CKC_R, KC_R, L_SYM)
        SMTD_MT(CKC_5, KC_5, KC_LEFT_SHIFT)
        SMTD_LT(CKC_Q, KC_Q, L_FN)
        SMTD_LT(CKC_Z, KC_Z, L_FR)
        SMTD_LT(CKC_UND, KC_UNDS, L_NUM)
    }
}

// Tap Dance declarations
enum {
    TPL,
};

// Tap Dance definitions
tap_dance_action_t tap_dance_actions[] = {
    // Tap once for Escape, twice for Caps Lock
    [TPL] = ACTION_TAP_DANCE_DOUBLE(KC_MINS, S(KC_EQL)),
};


const custom_shift_key_t custom_shift_keys[] = {
  {KC_DOT , US_AT}, // Shift . is @
  {KC_COMM, KC_COLN}, // Shift , is :
  {US_DQUO, KC_EQL }, // Shift " is =
  {KC_UNDS, KC_MINS }, // Shift _ is -
  {KC_LPRN, KC_LBRC }, // Shift ( is [
  {KC_RPRN, KC_RBRC }, // Shift ) is ]
  {KC_UNDS, KC_UNDS }, // Shift _ is _
  {KC_SLSH, US_ASTR }, // Shift / is *
  {US_RSQU, US_GRV }, // Shift ’ is `
  {KC_EXLM, KC_QUES}, // Shift ! is ?
  {US_HASH, US_SCLN},  // Shift # is ;
};
uint8_t NUM_CUSTOM_SHIFT_KEYS =
    sizeof(custom_shift_keys) / sizeof(custom_shift_key_t);

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
     /*
      * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
      * │Esc│ ’ │ B │ H │ G │ " │       │ # │ . │ / │ J │ X │ ! │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ ( │ C │ S │ N │ T │ K │       │ , │ A │ E │ I │ M │ ) │
      * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
      * │ [ │ P │ F │ L │ D │ V │       │ - │ U │ O │ Y │ W │ ] │
      * └───┴───┴───┴───┴───┴───┘       └───┴───┴───┴───┴───┴───┘
      *               ┌───┐                   ┌───┐
      *               │ Z ├───┐           ┌───┤ Q │
      *               └───┤ R ├───┐   ┌───┤   ├───┘
      *                   └───┤ _ │   │Esc├───┘
      *                       └───┘   └───┘
      */
    [L_BASE] = LAYOUT_split_3x6_3(
        KC_Z,    US_RSQU, KC_B,    KC_H,    KC_G,    US_DQUO,                                      US_HASH, KC_DOT,  KC_SLSH, KC_J,    KC_X,    CKC_QU,
        KC_LPRN, CKC_C,   CKC_S,   CKC_N,   CKC_T,   KC_K,                                         KC_COMM, CKC_A,   CKC_E,   CKC_I,   CKC_M,   KC_RPRN,
        KC_EQL,  KC_P,    KC_F,    KC_L,    KC_D,    KC_V,                                         KC_UNDS, KC_U,    KC_O,    KC_Y,    KC_W,    KC_EXLM,
                                            CKC_Z,   CKC_R, CKC_UND,                      KC_ESC,  CKC_SPC, CKC_Q
    ),
    [L_SE] = LAYOUT_split_3x6_3(
        _______, _______, _______, _______, _______, _______,                                      US_ARNG, _______, US_ADIA, _______, _______,  KC_Q,
        _______, _______, _______, _______, _______, _______,                                      _______, _______, _______, _______, _______,  _______,
        _______, _______, _______, _______, _______, _______,                                      US_ODIA, _______, _______, _______, _______,  _______,
                                            _______, _______, _______,                    _______, _______, _______
    ),
    [L_NUM] = LAYOUT_split_3x6_3(
        _______, KC_PSMS, KC_MINS, US_PLUS, KC_EQL,  _______,                                      _______, KC_DOT,  KC_SLSH, US_ASTR, _______, _______,
        _______,  KC_6,   KC_4,    KC_0,    KC_2,    _______,                                      _______, KC_3,    KC_1,    KC_5,    KC_7,    _______,
        US_DIV,   US_MUL,  _______, _______, KC_8,   _______,                                      KC_MINS, KC_9,    _______, _______, _______, _______,
                                            _______, _______, _______,                    _______, _______, _______
    ),
    [L_NAV] = LAYOUT_split_3x6_3(
        _______, _______, _______, _______, _______, _______,                                      KC_BRID, KC_PGDN, KC_UP,   KC_PGUP, KC_BRIU, KC_VOLU,
        _______, KC_LGUI, KC_LALT, KC_LSFT, KC_LCTL, S(KC_EQL),                                    KC_HOME, KC_LEFT, KC_DOWN, KC_RGHT, KC_END,  KC_VOLD,
        _______, _______, _______, _______, _______, _______,                                      _______, KC_MPRV, KC_MPLY, KC_MNXT, KC_MSTP, KC_MUTE,
                                            _______, _______, _______,                    _______, _______, _______
    ),
    [L_SYM] = LAYOUT_split_3x6_3(
        _______, US_QUOT, _______, KC_PIPE, US_PERC, _______,                                      _______, US_TILD, US_BSLS, _______, _______, _______,
        _______, KC_LABK, KC_LCBR, KC_RCBR, KC_RABK, _______,                                      _______, US_CIRC, CKC_DLR, US_EURO, _______, _______,
        _______, _______, KC_LBRC, KC_RBRC, _______, _______,                                      _______, US_AMPR, US_DEG,  _______, _______, _______,
                                            _______, _______, _______,                    _______, RALT(KC_SPC),  _______
    ),
    [L_FR] = LAYOUT_split_3x6_3(
        _______, _______, _______, _______, _______, US_DIAE,                                      US_AE,   U_ACRC,  U_ECRC,  U_ICRC,  US_MICR, US_NTIL,
        US_LDAQ, US_CCED, _______, _______, _______, _______,                                      U_AGRV,  _______, U_EGRV,  US_EACU, _______, US_RDAQ,
        US_IQUE, US_IEXL, _______, _______, _______, _______,                                      U_UCRC,  U_UGRV,  _______, U_OCRC,  US_OE,   US_OSTR,
                                            _______, _______, _______,                 RALT(KC_X), U_ELL,   _______
    ),
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
const uint16_t PROGMEM combo_backspace_se[] = {US_ADIA, KC_J, COMBO_END};
const uint16_t PROGMEM combo_delete[] = { KC_J, KC_X, COMBO_END};
const uint16_t PROGMEM combo_prtscr[] = {US_HASH, KC_DOT, COMBO_END};
const uint16_t PROGMEM combo_capsword[] = {KC_LPRN, KC_RPRN, COMBO_END};
const uint16_t PROGMEM combo_bootloader[] = {US_DQUO, US_HASH, COMBO_END};
const uint16_t PROGMEM combo_sleep[] = {KC_COMM, KC_MINS, COMBO_END};
const uint16_t PROGMEM combo_colon[] = {KC_P, KC_F, COMBO_END};
const uint16_t PROGMEM combo_enter[] = {CKC_A, CKC_I, COMBO_END};
const uint16_t PROGMEM combo_q[] = {KC_Y, KC_W, COMBO_END};
const uint16_t PROGMEM combo_q_se[] = {US_ODIA, KC_W, COMBO_END};
const uint16_t PROGMEM combo_se[] = {KC_SLSH, US_HASH, COMBO_END};
const uint16_t PROGMEM combo_se_back[] = {US_ARNG, US_ADIA, COMBO_END};
combo_t key_combos[] = {
    COMBO(combo_tab, KC_TAB),
    COMBO(combo_stab, S(KC_TAB)),
    COMBO(combo_backspace, KC_BSPC),
    COMBO(combo_backspace_se, KC_BSPC),
    COMBO(combo_delete, KC_DELETE),
    COMBO(combo_prtscr, KC_PRINT_SCREEN),
    COMBO(combo_capsword, CW_TOGG),
    COMBO(combo_bootloader, QK_BOOT),
    COMBO(combo_sleep, KC_SYSTEM_SLEEP),
    COMBO(combo_colon, KC_COLN),
    COMBO(combo_enter, KC_ENT),
    COMBO(combo_q, KC_Q),
    COMBO(combo_q_se, KC_Q),
    COMBO(combo_se, TG(L_SE)),
    COMBO(combo_se_back, TG(L_SE)),
};
