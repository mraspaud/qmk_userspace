#pragma once

// Handedness
#undef SPLIT_HAND_PIN
#define EE_HANDS

// Lighting
#define SPLIT_LAYER_STATE_ENABLE
#define SPLIT_TRANSACTION_IDS_USER SHORE_SYNC
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
// Not NO_USB_STARTUP_CHECK: on ChibiOS it compiles out the USB suspend loop,
// which is the only place that sends remote wakeup, so the keyboard could not
// wake the computer. The secondary half is unaffected: its USB is stopped, so
// it never enters that loop.

// Enables the combo_should_trigger() hook below (used to guard the layer
// combos against misfires while typing, and to delay the sym backspace combo).
#define COMBO_SHOULD_TRIGGER

// Cold boot: master/slave is chosen by split_role.h (included in keymap.c),
// which waits for the USB host as long as needed, and starts the secondary as
// soon as the master talks to it. SPLIT_USB_TIMEOUT is therefore unused.
// The watchdog is only a safety net: it reboots a half that elected itself
// secondary but never hears from a master (e.g. noise taken for traffic).
// A correct secondary is pinged by the master within ~0.5 s, after which the
// watchdog never fires, not even while the computer sleeps.
#define SPLIT_WATCHDOG_ENABLE
