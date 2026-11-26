// Host-only test of the split master election in split_role.h.
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#define SOFT_SERIAL_PIN 13
#include "gpio.h"
#include "timer.h"
#include "usb_util.h"

// Simulated time advances by 1 us on every pin read and 10 us on every timer read.
static uint64_t now_us;
static uint64_t usb_active_at_us; // UINT64_MAX: never
static uint64_t master_traffic_at_us; // UINT64_MAX: never
static uint64_t glitch_at_us; // a single low pulse, UINT64_MAX: none
static bool usb_disconnected;
static int configured_pin = -1;

uint32_t timer_read32(void) { now_us += 10; return (uint32_t)(now_us / 1000); }
uint32_t timer_elapsed32(uint32_t t) { return timer_read32() - t; }
bool usb_connected_state(void) { return now_us >= usb_active_at_us; }
void usb_disconnect(void) { usb_disconnected = true; }
void gpio_set_pin_input_high(pin_t pin) { configured_pin = pin; }
bool gpio_read_pin(pin_t pin) {
    assert(pin == SOFT_SERIAL_PIN);
    now_us += 1;
    // Power-up: low until the pull-ups win at 5 ms.
    if (now_us < 5000) return false;
    if (glitch_at_us != UINT64_MAX && now_us >= glitch_at_us && now_us < glitch_at_us + 3) return false;
    if (now_us >= master_traffic_at_us) {
        // One handshake byte (~20 us low) every 2 ms, like the master's retries.
        return ((now_us - master_traffic_at_us) % 2000) >= 20;
    }
    return true;
}

#include "../split_role.h"

static bool run(uint64_t usb_at, uint64_t traffic_at, uint64_t glitch_at) {
    now_us = 0;
    usb_active_at_us = usb_at;
    master_traffic_at_us = traffic_at;
    glitch_at_us = glitch_at;
    usb_disconnected = false;
    return is_keyboard_master_impl();
}

int main(void) {
    const uint64_t NEVER = UINT64_MAX;

    // Host ready quickly: master, USB left connected.
    assert(run(300000, NEVER, NEVER));
    assert(!usb_disconnected && configured_pin == SOFT_SERIAL_PIN);

    // Host takes 45 s (slower than any SPLIT_USB_TIMEOUT used so far): still master.
    assert(run(45000000ULL, NEVER, NEVER));
    assert(!usb_disconnected);

    // Other half becomes master at 1.2 s: secondary within a few ms, USB disconnected.
    assert(!run(NEVER, 1200000, NEVER));
    assert(usb_disconnected);
    assert(now_us < 1200000 + 10000);

    // Master already running when this half boots: secondary just after settling.
    assert(!run(NEVER, 0, NEVER));
    assert(usb_disconnected && now_us < 70000);

    // Power-up low level and one later glitch are not mistaken for a master.
    assert(run(2000000, NEVER, 800000));

    printf("PASS: fast host, 45 s host, late master, running master, glitch rejection\n");
    return 0;
}
