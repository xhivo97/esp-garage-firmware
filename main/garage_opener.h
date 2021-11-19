#pragma once

#include "driver/gpio.h"
#include "driver/timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include <freertos/queue.h>

#define LIMIT_A_GPIO    0
#define LIMIT_B_GPIO    1
#define RELAY_A_GPIO    4
#define RELAY_B_GPIO    5
#define LIGHT_GPIO      6
#define BUTTON_GPIO     7

#define GPIO_OUTPUT_PIN_SEL ((1ULL<<RELAY_A_GPIO) | (1ULL<<RELAY_B_GPIO) | (1ULL<<LIGHT_GPIO))
#define GPIO_INPUT_PIN_SEL  ((1ULL<<LIMIT_A_GPIO) | (1ULL<<LIMIT_B_GPIO) | (1ULL<<BUTTON_GPIO))
#define ESP_INTR_FLAG_DEFAULT 0

#define GARAGE_OPEN_ALERT_MINUTES   15
#define LIGHT_ON_MINUTES            2
#define MOTOR_RUN_LIMIT_SECONDS     30

enum garage_states {
  IDLE_S,
  OPENED_S,
  CLOSED_S,
  OPENING_S,
  CLOSING_S,
  STOPPED_WHILE_OPENING_S,
  STOPPED_WHILE_CLOSING_S,
  FLOATING_S,
  LIMIT_SWITCH_ERROR_S
};

void lightOffCallback();
void garageOpenCallback();
void debounceCallback();
void setup_gpios();
void light_on();
void garage_open();
void garage_close();
void garage_stop();
void run_state_machine();
char* get_garage_state();
void queue_state_machine();