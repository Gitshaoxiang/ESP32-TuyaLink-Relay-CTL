#include "relay_ctl.h"

void init_relay_io(gpio_num_t pin) {
    gpio_reset_pin(pin);
    gpio_set_direction(pin , GPIO_MODE_OUTPUT);
}

void set_relay_status(gpio_num_t pin, bool status){
    gpio_set_level(pin, status);
}
