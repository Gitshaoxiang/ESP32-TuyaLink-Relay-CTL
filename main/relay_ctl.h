#include "driver/gpio.h"

#define RELAY_PIN GPIO_NUM_12
#define SWITCH_DP_ID_KEY "socket_1"

void init_relay_io(gpio_num_t pin);
void set_relay_status(gpio_num_t pin, bool status);
