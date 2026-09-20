#include "mbr/hat/hat.h"

#include <stddef.h>
#include <stdint.h>

#ifdef MBR_PLATFORM_PICO
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#endif

typedef struct { mbr_control_t control; uint8_t pin; } mbr_hat_pin_map_t;

static const mbr_hat_pin_map_t k_pin_map[] = {
    {MBR_CONTROL_JOY_UP,MBR_HAT_PIN_JOY_UP},
    {MBR_CONTROL_JOY_DOWN,MBR_HAT_PIN_JOY_DOWN},
    {MBR_CONTROL_JOY_LEFT,MBR_HAT_PIN_JOY_LEFT},
    {MBR_CONTROL_JOY_RIGHT,MBR_HAT_PIN_JOY_RIGHT},
    {MBR_CONTROL_JOY_PRESS,MBR_HAT_PIN_JOY_PRESS},
    {MBR_CONTROL_KEY_A,MBR_HAT_PIN_KEY_A},
    {MBR_CONTROL_KEY_B,MBR_HAT_PIN_KEY_B},
    {MBR_CONTROL_KEY_X,MBR_HAT_PIN_KEY_X},
    {MBR_CONTROL_KEY_Y,MBR_HAT_PIN_KEY_Y}
};

bool mbr_hat_control_for_pin(uint8_t pin,mbr_control_t *control)
{
    if (control==NULL) return false;
    for (size_t i=0u;i<sizeof(k_pin_map)/sizeof(k_pin_map[0]);++i)
        if (k_pin_map[i].pin==pin) { *control=k_pin_map[i].control; return true; }
    return false;
}

uint8_t mbr_hat_pin_for_control(mbr_control_t control)
{
    for (size_t i=0u;i<sizeof(k_pin_map)/sizeof(k_pin_map[0]);++i)
        if (k_pin_map[i].control==control) return k_pin_map[i].pin;
    return UINT8_MAX;
}

#ifdef MBR_PLATFORM_PICO
#define MBR_HAT_DEBOUNCE_MS 20u
#define MBR_HAT_SCAN_PERIOD_MS 1u
#define MBR_HAT_EVENT_QUEUE_LENGTH 32u

typedef struct {
    mbr_control_t control;
    uint8_t pin;
    bool raw_pressed;
    bool stable_pressed;
    uint32_t raw_changed_ms;
} mbr_hat_input_state_t;

static mbr_hat_input_state_t g_inputs[MBR_CONTROL_COUNT];
static mbr_hat_event_t g_events[MBR_HAT_EVENT_QUEUE_LENGTH];
static uint8_t g_read_index;
static uint8_t g_write_index;
static uint32_t g_last_scan_ms;

static uint32_t now_ms(void) { return to_ms_since_boot(get_absolute_time()); }

static bool enqueue(mbr_control_t control,bool pressed)
{
    const uint8_t next=(uint8_t)((g_write_index+1u)%MBR_HAT_EVENT_QUEUE_LENGTH);
    if (next==g_read_index) return false;
    g_events[g_write_index].control=control;
    g_events[g_write_index].pressed=pressed;
    g_write_index=next;
    return true;
}

void mbr_hat_pico_init(void)
{
    const uint32_t now=now_ms();
    for (uint8_t i=0u;i<MBR_CONTROL_COUNT;++i) {
        mbr_hat_input_state_t *input=&g_inputs[i];
        input->control=(mbr_control_t)i;
        input->pin=mbr_hat_pin_for_control(input->control);
        gpio_init(input->pin);
        gpio_set_dir(input->pin,GPIO_IN);
        gpio_pull_up(input->pin);
        input->raw_pressed=!gpio_get(input->pin);
        input->stable_pressed=input->raw_pressed;
        input->raw_changed_ms=now;
    }
    g_read_index=0u;
    g_write_index=0u;
    g_last_scan_ms=now;
}

void mbr_hat_pico_task(void)
{
    const uint32_t now=now_ms();
    if ((uint32_t)(now-g_last_scan_ms)<MBR_HAT_SCAN_PERIOD_MS) return;
    g_last_scan_ms=now;
    for (uint8_t i=0u;i<MBR_CONTROL_COUNT;++i) {
        mbr_hat_input_state_t *input=&g_inputs[i];
        const bool pressed=!gpio_get(input->pin);
        if (pressed!=input->raw_pressed) {
            input->raw_pressed=pressed;
            input->raw_changed_ms=now;
        }
        if (pressed!=input->stable_pressed && (uint32_t)(now-input->raw_changed_ms)>=MBR_HAT_DEBOUNCE_MS) {
            input->stable_pressed=pressed;
            (void)enqueue(input->control,pressed);
        }
    }
}

bool mbr_hat_pico_poll_event(mbr_hat_event_t *event)
{
    if (event==NULL || g_read_index==g_write_index) return false;
    *event=g_events[g_read_index];
    g_read_index=(uint8_t)((g_read_index+1u)%MBR_HAT_EVENT_QUEUE_LENGTH);
    return true;
}
#else
void mbr_hat_pico_init(void) {}
void mbr_hat_pico_task(void) {}
bool mbr_hat_pico_poll_event(mbr_hat_event_t *event) { (void)event; return false; }
#endif