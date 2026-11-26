// Copyright 2026 Martin Raspaud (@mraspaud)
// SPDX-License-Identifier: GPL-2.0
//
// Split master election that survives slow cold boots.
//
// QMK's default (SPLIT_USB_DETECT) gives each half SPLIT_USB_TIMEOUT to see an
// enumerated USB host, and a half that does not see one becomes the secondary
// for good. When the computer enumerates USB later than that (cold boot, GRUB),
// both halves end up secondary and the keyboard is dead until it is replugged.
// Raising the timeout fixes it, but the real secondary then always waits the
// whole timeout before it starts.
//
// This replaces QMK's weak is_keyboard_master_impl(). Each half waits, with no
// timeout, for one of two things:
//   - its own USB reaching USB_ACTIVE (enumerated by the host): it is master;
//   - the master's split-serial traffic on the split line: it is secondary.
// The master starts its serial transactions right after deciding, and each
// transaction attempt starts with a byte, so the line (idle high, pulled up on
// both sides) goes low. The secondary sees that within a few ms and starts.
// The secondary never transmits on its own, so the USB half never mistakes it
// for a master.
#pragma once

#include "gpio.h"
#include "timer.h"
#include "usb_util.h"

// The pin on which this half receives the other half's transmissions: the
// shared line in half duplex (Cantor Pro: one pin per half), RX in full duplex.
#ifndef SPLIT_ROLE_PIN
#    if defined(SERIAL_USART_FULL_DUPLEX)
#        define SPLIT_ROLE_PIN SERIAL_USART_RX_PIN
#    elif defined(SOFT_SERIAL_PIN)
#        define SPLIT_ROLE_PIN SOFT_SERIAL_PIN
#    elif defined(SERIAL_USART_TX_PIN)
#        define SPLIT_ROLE_PIN SERIAL_USART_TX_PIN
#    else
#        error "split_role.h needs the split serial pin (SOFT_SERIAL_PIN or SERIAL_USART_TX_PIN)."
#    endif
#endif

// Ignore the line while both halves come out of reset and set their pull-ups
// (RP2040 pads default to pull-down), so that this does not count as traffic.
#ifndef SPLIT_ROLE_SETTLE_MS
#    define SPLIT_ROLE_SETTLE_MS 50
#endif
// Falling edges needed to decide that the other half is master. Its first
// attempts come every few ms, then every 500 ms once it counts us as absent.
#ifndef SPLIT_ROLE_MIN_EDGES
#    define SPLIT_ROLE_MIN_EDGES 3
#endif
// 0 waits forever, which is what a slow host needs. If set, a half that has
// seen neither USB nor a master by then becomes secondary, like stock QMK.
#ifndef SPLIT_ROLE_TIMEOUT_MS
#    define SPLIT_ROLE_TIMEOUT_MS 0
#endif

bool is_keyboard_master_impl(void) {
    gpio_set_pin_input_high(SPLIT_ROLE_PIN);
    const uint32_t start = timer_read32();
    bool settled = false;
    bool last_level = true;
    uint8_t edges = 0;

    while (true) {
        if (usb_connected_state()) {
            return true;
        }
        const uint32_t elapsed = timer_elapsed32(start);
        if (!settled) {
            settled = elapsed >= SPLIT_ROLE_SETTLE_MS;
            last_level = gpio_read_pin(SPLIT_ROLE_PIN);
            continue;
        }
        // Sample the pin many times per timer/USB check: a start bit is only
        // ~2 us at the default 460800 baud.
        for (uint8_t i = 0; i < 64; i++) {
            const bool level = gpio_read_pin(SPLIT_ROLE_PIN);
            if (last_level && !level) {
                edges++;
            }
            last_level = level;
        }
        if (edges >= SPLIT_ROLE_MIN_EDGES) {
            break;
        }
#if SPLIT_ROLE_TIMEOUT_MS > 0
        if (elapsed >= SPLIT_ROLE_TIMEOUT_MS) {
            break;
        }
#endif
    }
    // Same as QMK: the secondary must not stay visible on USB.
    usb_disconnect();
    return false;
}
