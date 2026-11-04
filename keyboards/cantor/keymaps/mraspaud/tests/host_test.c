// Host-only smoke test of the animation's pure computation and USB gating.
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
typedef uint32_t layer_state_t;
#define L_BASE 0
#define L_EN 1
#define L_SE 3
#define L_FR 4
#define L_NUMSYM 5
#define L_NAV 6
#define L_FRSYM 7
static inline bool layer_state_cmp(layer_state_t s,int layer) {return (s & (1u<<layer)) !=0;}
static uint32_t mock_ms;
static bool mock_master=true,mock_left=true,mock_connected=false;
static int mock_usb_state=1, mock_flushes=0;
static uint8_t mock_leds[7][3];
static void set_c(int i,uint8_t r,uint8_t g,uint8_t b) {assert(i>=0 && i<7);mock_leds[i][0]=r;mock_leds[i][1]=g;mock_leds[i][2]=b;}
static void set_all(uint8_t r,uint8_t g,uint8_t b) {for(int i=0;i<7;i++)set_c(i,r,g,b);}
static void flush(void) {mock_flushes++;}
#include "rgblight_drivers.h"
const rgblight_driver_t rgblight_driver={set_c,set_all,flush};
void rgblight_enable_noeeprom(void) {}
void rgblight_mode_noeeprom(int mode) {(void)mode;}
uint32_t sync_timer_read32(void) {return mock_ms;}
uint32_t timer_read32(void) {return mock_ms;}
uint32_t timer_elapsed32(uint32_t t) {return mock_ms-t;}
bool is_keyboard_master(void) {return mock_master;}
bool is_keyboard_left(void) {return mock_left;}
bool is_transport_connected(void) {return mock_connected;}
int usb_device_state_get_configure_state(void) {return mock_usb_state;}
void transaction_register_rpc(int id,void(*cb)(uint8_t,const void*,uint8_t,void*)) {(void)id;(void)cb;}
bool transaction_rpc_send(int id,uint8_t size,const void *arg) {(void)id;(void)size;(void)arg;return true;}
#include "../shore_lighting.h"
static int sum(void) {int s=0;for(int i=0;i<7;i++)for(int j=0;j<3;j++)s+=mock_leds[i][j];return s;}
int main(void) {
    shore_init();
    assert(sum()==0);
    for(mock_ms=0;mock_ms<500;mock_ms+=34)shore_task(0);
    assert(sum()==0); // powered != configured
    mock_usb_state=USB_DEVICE_STATE_CONFIGURED;
    shore_task(0);
    assert(sum()==0); // first instant still dark
    for(mock_ms=510;mock_ms<23000;mock_ms+=34)shore_task(0);
    assert(sum()>0); // normal Shore now alive
    shore_task(0); // allow any frame already due at this timestamp
    int before=mock_flushes;
    for(int i=0;i<100;i++)shore_task(0);
    assert(mock_flushes==before); // no writes faster than 30fps
    int before2=mock_flushes;
    mock_ms+=34;
    shore_task(1u<<L_SE);
    assert(mock_flushes==before2+1);
    shore_task((1u<<L_SE)|(1u<<L_NAV));
    mock_ms+=200;
    shore_task((1u<<L_SE)|(1u<<L_NAV));
    mock_ms+=250;
    shore_task((1u<<L_SE)|(1u<<L_NAV));
    assert(mock_leds[6][0] || mock_leds[6][1] || mock_leds[6][2]);
    mock_usb_state=3;
    mock_ms+=34;
    shore_task(0);
    assert(sum()==0);
    printf("PASS: powered state, initial darkness, animation, capped fps, layered top, suspend darkness (%d flushes)\n", mock_flushes);
    return 0;
}
