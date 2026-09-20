#include "mbr/hat/hat.h"
#include "hardware/gpio.h"
void mbr_hat_init(void) { for(unsigned i=0;i<MBR_CONTROL_COUNT;++i) { unsigned pin=mbr_hat_pins[i];gpio_init(pin);gpio_set_dir(pin,GPIO_IN);gpio_pull_up(pin); } }
uint16_t mbr_hat_read(void) { uint16_t bits=0;for(unsigned i=0;i<MBR_CONTROL_COUNT;++i) if(!gpio_get(mbr_hat_pins[i])) bits|=(uint16_t)(1u<<i);return bits; }
