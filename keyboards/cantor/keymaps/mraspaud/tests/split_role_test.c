// Host-only test of the split master election in split_role.h.
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#define SOFT_SERIAL_PIN 13
#include "gpio.h"
#include "timer.h"
#include "usb_util.h"

#define NEVER UINT64_MAX
#define MS 1000ULL
#define S 1000000ULL

// Simulated time advances by 1 us on every pin read and 10 us on every timer read.
static uint64_t now_us;
static uint64_t usb_active_at_us;
static uint64_t master_at_us; // when the other half starts its master traffic
static const uint64_t *glitches; // start times of noise, each a 3 us low pulse
static uint64_t bounce_at_us, bounce_len_us; // contact bounce: line toggling
static bool usb_disconnected;
static int configured_pin = -1;

uint32_t timer_read32(void) { now_us += 10; return (uint32_t)(now_us / 1000); }
uint32_t timer_elapsed32(uint32_t t) { return timer_read32() - t; }
bool usb_connected_state(void) { return now_us >= usb_active_at_us; }
void usb_disconnect(void) { usb_disconnected = true; }
void gpio_set_pin_input_high(pin_t pin) { configured_pin = pin; }

// The master's attempts while we do not answer: one handshake byte 0x00 (line
// low for ~20 us), 10 tries ~22 ms apart, then one every 500 ms.
static bool master_low(uint64_t t) {
    if (t < master_at_us) return false;
    uint64_t d = t - master_at_us;
    uint64_t start;
    if (d < 10 * 22 * MS) {
        start = d / (22 * MS) * (22 * MS);
    } else {
        start = 10 * 22 * MS + (d - 10 * 22 * MS) / (500 * MS) * (500 * MS);
    }
    return d - start < 20;
}

bool gpio_read_pin(pin_t pin) {
    assert(pin == SOFT_SERIAL_PIN);
    now_us += 1;
    if (now_us < 5 * MS) return false; // power-up: low until the pull-ups win
    for (const uint64_t *g = glitches; g && *g != NEVER; g++) {
        if (now_us >= *g && now_us < *g + 3) return false;
    }
    if (now_us >= bounce_at_us && now_us < bounce_at_us + bounce_len_us) {
        return (now_us / 7) % 2; // toggles every 7 us
    }
    return !master_low(now_us);
}

#include "../split_role.h"

static const uint64_t no_glitches[] = {NEVER};

static bool run(uint64_t usb_at, uint64_t master_at, const uint64_t *g, uint64_t bounce_at, uint64_t bounce_len) {
    now_us = 0;
    usb_active_at_us = usb_at;
    master_at_us = master_at;
    glitches = g ? g : no_glitches;
    bounce_at_us = bounce_at;
    bounce_len_us = bounce_len;
    usb_disconnected = false;
    return is_keyboard_master_impl();
}

int main(void) {
    // Host ready quickly: master, USB left connected.
    assert(run(300 * MS, NEVER, NULL, NEVER, 0));
    assert(!usb_disconnected && configured_pin == SOFT_SERIAL_PIN);

    // Host takes 45 s: still master.
    assert(run(45 * S, NEVER, NULL, NEVER, 0));
    assert(!usb_disconnected);

    // Other half becomes master at 1.2 s: secondary after 3 attempts (~50 ms).
    assert(!run(NEVER, 1200 * MS, NULL, NEVER, 0));
    assert(usb_disconnected && now_us < 1200 * MS + 100 * MS);

    // Master already in its 500 ms phase when we boot: secondary within ~1.5 s.
    assert(!run(NEVER, 0, NULL, NEVER, 0));
    assert(usb_disconnected && now_us > 200 * MS && now_us < 2 * S);

    // Isolated glitches seconds apart never add up (powered long before the host).
    static const uint64_t sparse[] = {1 * S, 3 * S, 5 * S, 7 * S, 9 * S, NEVER};
    assert(run(10 * S, NEVER, sparse, NEVER, 0));

    // A 30 ms contact bounce (TRRS plugged during boot) counts once.
    assert(run(3 * S, NEVER, NULL, 1 * S, 30 * MS));

    // A bounce, then a glitch 800 ms later: the pause restarts the count.
    static const uint64_t late[] = {1800 * MS, NEVER};
    assert(run(3 * S, NEVER, late, 1 * S, 30 * MS));

    // Known limit: noise spaced like master attempts (3 events, 100-700 ms
    // apart) is taken for a master. SPLIT_WATCHDOG_ENABLE then reboots the half.
    static const uint64_t like_master[] = {1300 * MS, 1600 * MS, NEVER};
    assert(!run(3 * S, NEVER, like_master, 1 * S, 30 * MS));

    printf("PASS: fast host, 45 s host, late master, running master, sparse glitches, contact bounce, documented limit\n");
    return 0;
}
