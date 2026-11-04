#pragma once

// Handedness
#undef SPLIT_HAND_PIN
#define EE_HANDS

// Lighting
// #define RGBLIGHT_LAYERS
// #define RGBLIGHT_LAYERS_OVERRIDE_RGB_OFF
// #define RGBLIGHT_DEFAULT_ON false
#define SPLIT_LAYER_STATE_ENABLE
#undef RGBLIGHT_LED_COUNT
#define RGBLIGHT_LED_COUNT 14
#undef RGBLED_SPLIT
#define RGBLED_SPLIT { 7, 7 }

#define TAPPING_TERM 250
#define PERMISSIVE_HOLD
#define FLOW_TAP_TERM 150
#define CHORDAL_HOLD
#define SPECULATIVE_HOLD
#define UNICODE_SELECTED_MODES UNICODE_MODE_LINUX
#define UNICODE_KEY_LNX LCTL(LSFT(KC_U))
#define UNICODE_TYPE_DELAY 10
#define MAX_DEFERRED_EXECUTORS 10
#define USB_SUSPEND_WAKEUP_DELAY 200
#define NO_USB_STARTUP_CHECK

// Enables the combo_should_trigger() hook below (used to guard the layer
// combos against misfires while typing, and to delay the sym backspace combo).
#define COMBO_SHOULD_TRIGGER
