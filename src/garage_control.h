#ifndef GARAGE_CONTROL_H
#define GARAGE_CONTROL_H

#include "driver/gpio.h"
#include "driver/timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define LIMIT_A_GPIO    0 // Limit switch A GPIO pin (INPUT)
#define LIMIT_B_GPIO    1 // Limit switch B GPIO pin (INPUT)
#define RELAY_A_GPIO    4 // Relay A GPIO pin (OUTPUT)
#define RELAY_B_GPIO    5 // Relay B GPIO pin (OUTPUT)
#define LIGHT_GPIO      6 // Light control GPIO pin (OUTPUT)
#define BUTTON_GPIO     7 // User button GPIO pin (INPUT)

#define ACTIVE_LOW              1 // Enable active low input
#define INVERT_MOTOR_DIRECTION  0 // Invert the motor direction
#define INVERT_LIMIT_SWITCHES   0 // Invert the limit switches

// Output GPIOs selection mask
#define GPIO_OUTPUT_PIN_SEL ((1ULL<<RELAY_A_GPIO) | (1ULL<<RELAY_B_GPIO) | (1ULL<<LIGHT_GPIO))
// Input GPIOs selection mask
#define GPIO_INPUT_PIN_SEL  ((1ULL<<LIMIT_A_GPIO) | (1ULL<<LIMIT_B_GPIO) | (1ULL<<BUTTON_GPIO))

// Sets polling rate to 100Hz
#define POLL_TICK_MS        10
#define MIN_TO_TICKS(x)     ((x * 60000) / POLL_TICK_MS)
#define SEC_TO_TICKS(x)     ((x * 1000) / POLL_TICK_MS)
#define MILLIS_TO_TICKS(x)  (x / POLL_TICK_MS)
#define TICKS_TO_MILLIS(x)  (x * POLL_TICK_MS)

#define DOOR_OPEN_NOTIFICATION_MINUTES 15

static const char *TAG = "app_main";

extern void update_status(char *state);
extern void send_notification(char *message);

inline __attribute__((always_inline)) int get_level(gpio_num_t gpio_num) {
    return (gpio_get_level(gpio_num) ^ ACTIVE_LOW);
}

inline __attribute__((always_inline)) void open_garage() {
    gpio_set_level(RELAY_A_GPIO, 1 ^ INVERT_MOTOR_DIRECTION);
    gpio_set_level(RELAY_B_GPIO, 0 ^ INVERT_MOTOR_DIRECTION);
}
inline __attribute__((always_inline)) void close_garage() {
    gpio_set_level(RELAY_A_GPIO, 0 ^ INVERT_MOTOR_DIRECTION);
    gpio_set_level(RELAY_B_GPIO, 1 ^ INVERT_MOTOR_DIRECTION);
}
inline __attribute__((always_inline)) void stop_garage() {
    gpio_set_level(RELAY_A_GPIO, 0);
    gpio_set_level(RELAY_B_GPIO, 0);
}

static uint32_t current_tick = 0;

enum garage_timer {
    LIM_SW_A_DEBOUNCE_TIMER,
    LIM_SW_B_DEBOUNCE_TIMER,
    BUTTON_DEBOUNCE_TIMER,
    LIGHT_ON_TIMER,
    MOTOR_RUN_LIMIT_TIMER,
    GARAGE_OPEN_TIMER,
    SW_BUTTON_COOLDOWN_TIMER,
};

static uint32_t timer_duration[] = {
    [LIM_SW_A_DEBOUNCE_TIMER]   = MILLIS_TO_TICKS(20),
    [LIM_SW_B_DEBOUNCE_TIMER]   = MILLIS_TO_TICKS(20),
    [BUTTON_DEBOUNCE_TIMER]     = MILLIS_TO_TICKS(20),
    [LIGHT_ON_TIMER]            = MIN_TO_TICKS(2),
    [MOTOR_RUN_LIMIT_TIMER]     = SEC_TO_TICKS(30),
    [GARAGE_OPEN_TIMER]         = MIN_TO_TICKS(DOOR_OPEN_NOTIFICATION_MINUTES),
    [SW_BUTTON_COOLDOWN_TIMER]  = MILLIS_TO_TICKS(500),
};

static uint32_t start_tick[] = {
    [LIM_SW_A_DEBOUNCE_TIMER]   = 0,
    [LIM_SW_B_DEBOUNCE_TIMER]   = 0,
    [BUTTON_DEBOUNCE_TIMER]     = 0,
    [LIGHT_ON_TIMER]            = 0,
    [MOTOR_RUN_LIMIT_TIMER]     = 0,
    [GARAGE_OPEN_TIMER]         = 0,
    [SW_BUTTON_COOLDOWN_TIMER]  = 0,
};

bool timer_enabled[] = {
    [LIM_SW_A_DEBOUNCE_TIMER]   = false,
    [LIM_SW_B_DEBOUNCE_TIMER]   = false,
    [BUTTON_DEBOUNCE_TIMER]     = false,
    [LIGHT_ON_TIMER]            = false,
    [MOTOR_RUN_LIMIT_TIMER]     = false,
    [GARAGE_OPEN_TIMER]         = false,
    [SW_BUTTON_COOLDOWN_TIMER]  = false,
};

static const char * const timer_names[] = {
    [LIM_SW_A_DEBOUNCE_TIMER]   = "TIMER_LIM_SW_A_DEBOUNCE_TIMER",
    [LIM_SW_B_DEBOUNCE_TIMER]   = "TIMER_LIM_SW_B_DEBOUNCE_TIMER",
    [BUTTON_DEBOUNCE_TIMER]     = "TIMER_BUTTON_DEBOUNCE_TIMER",
    [LIGHT_ON_TIMER]            = "TIMER_LIGHT_ON_TIMER",
    [MOTOR_RUN_LIMIT_TIMER]     = "TIMER_MOTOR_RUN_LIMIT_TIMER",
    [GARAGE_OPEN_TIMER]         = "TIMER_GARAGE_OPEN_TIMER",
    [SW_BUTTON_COOLDOWN_TIMER]  = "TIMER_SW_BUTTON_COOLDOWN_TIMER",
};

bool prev_input[] = {
    [LIM_SW_A_DEBOUNCE_TIMER]   = false,
    [LIM_SW_B_DEBOUNCE_TIMER]   = false,
    [BUTTON_DEBOUNCE_TIMER]     = false,
};

const int NUM_TIMERS = sizeof(timer_duration) / sizeof(uint32_t);

enum garage_states {
    IDLE_S,
    OPENED_S,
    CLOSED_S,
    OPENING_S,
    CLOSING_S,
    STOPPED_WHILE_OPENING_S,
    STOPPED_WHILE_CLOSING_S,
    FLOATING_S,
    INVALID_LIMIT_SW_S,
};

static char * const garage_state_name[] = {
    [IDLE_S]                  = "IDLE",
    [OPENED_S]                = "OPENED",
    [CLOSED_S]                = "CLOSED",
    [OPENING_S]               = "OPENING",
    [CLOSING_S]               = "CLOSING",
    [STOPPED_WHILE_OPENING_S] = "STOPPED WHILE OPENING",
    [STOPPED_WHILE_CLOSING_S] = "STOPPED WHILE CLOSING",
    [FLOATING_S]              = "FLOATING",
    [INVALID_LIMIT_SW_S]      = "INVALID LIMIT SWITCH STATE",
};

// Current state of the garage logic state machine
enum garage_states garage_state      = IDLE_S;
// Previous state of the garage logic state machine
enum garage_states garage_prev_state = IDLE_S;

enum garage_states init_garage_state() {
    if (!get_level(LIMIT_A_GPIO) && !get_level(LIMIT_B_GPIO))
        return FLOATING_S;

    if (get_level(LIMIT_A_GPIO) && !get_level(LIMIT_B_GPIO))
        return INVERT_LIMIT_SWITCHES ? CLOSED_S : OPENED_S;

    if (get_level(LIMIT_B_GPIO) && !get_level(LIMIT_A_GPIO))
        return INVERT_LIMIT_SWITCHES ? OPENED_S : CLOSED_S;

    return INVALID_LIMIT_SW_S; 
}

static void event_listener() {
    // Wait for esp rainmaker to create the needed params
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    for (;;) {
        vTaskDelay(POLL_TICK_MS / portTICK_PERIOD_MS);
        current_tick++;

        timer_enabled[LIM_SW_A_DEBOUNCE_TIMER]  = get_level(LIMIT_A_GPIO);
        timer_enabled[LIM_SW_B_DEBOUNCE_TIMER]  = get_level(LIMIT_B_GPIO);
        timer_enabled[BUTTON_DEBOUNCE_TIMER]    = get_level(BUTTON_GPIO);
        timer_enabled[MOTOR_RUN_LIMIT_TIMER]    = garage_state == OPENING_S || garage_state == CLOSING_S;
        timer_enabled[GARAGE_OPEN_TIMER]        = garage_state != CLOSED_S;

        for (int i = 0; i < NUM_TIMERS; i++) {
            if (!timer_enabled[i]) {
                start_tick[i] = current_tick;
                continue;
            }
            if ((current_tick - start_tick[i] == timer_duration[i])) {
                ESP_LOGI(TAG, "%-29s -> %ums",
                    timer_names[i], TICKS_TO_MILLIS(timer_duration[i]));
                timer_enabled[i] = false;
                switch (i) {
                    case BUTTON_DEBOUNCE_TIMER:
                    case SW_BUTTON_COOLDOWN_TIMER:
                        // Update the light timer
                        start_tick[LIGHT_ON_TIMER] = current_tick;
                        timer_enabled[LIGHT_ON_TIMER] = true;
                        gpio_set_level(LIGHT_GPIO, 1);
                        switch (garage_state) {
                            case OPENED_S:
                            case STOPPED_WHILE_OPENING_S:
                                garage_state = CLOSING_S;
                                close_garage();
                                break;
                            case CLOSED_S:
                            case STOPPED_WHILE_CLOSING_S:
                            case FLOATING_S:
                                garage_state = OPENING_S;
                                open_garage();
                                break;
                            case OPENING_S:
                                if (get_level(LIMIT_A_GPIO) || get_level(LIMIT_B_GPIO))
                                    break;
                                garage_state = STOPPED_WHILE_OPENING_S;
                                stop_garage();
                                break;
                            case CLOSING_S:
                                if (get_level(LIMIT_A_GPIO) || get_level(LIMIT_B_GPIO))
                                    break;
                                garage_state = STOPPED_WHILE_CLOSING_S;
                                stop_garage();
                                break;
                            default:
                                break;
                        }
                        break;
                    case MOTOR_RUN_LIMIT_TIMER:
                        stop_garage();
                        if (garage_state == OPENING_S)
                            garage_state = CLOSED_S;
                        else
                            garage_state = OPENED_S;
                        send_notification("Motor ran for longer than expected.");
                        break;
                    case LIGHT_ON_TIMER:
                        gpio_set_level(LIGHT_GPIO, 0);
                        break;
                    case LIM_SW_A_DEBOUNCE_TIMER:
                        stop_garage();
                        garage_state = INVERT_LIMIT_SWITCHES ? CLOSED_S : OPENED_S;
                        break;
                    case LIM_SW_B_DEBOUNCE_TIMER:
                        stop_garage();
                        garage_state = INVERT_LIMIT_SWITCHES ? OPENED_S : CLOSED_S;
                        break;
                    case GARAGE_OPEN_TIMER:
                        {
                            char *minute = DOOR_OPEN_NOTIFICATION_MINUTES > 1 ? "minutes" : "minute";
                            char message[128];
                            sprintf(message,"Garage has been open for %d %s",
                                DOOR_OPEN_NOTIFICATION_MINUTES, minute);
                            send_notification(message);
                        }
                        break;
                }
            }
        }
        if (garage_prev_state != garage_state) {
            ESP_LOGI(TAG, "%-29s -> %s", garage_state_name[garage_prev_state], garage_state_name[garage_state]);
            garage_prev_state = garage_state;
            update_status(garage_state_name[garage_state]);
        }
    }
}

esp_err_t garage_init() {
    gpio_config_t out_gpio = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = GPIO_OUTPUT_PIN_SEL,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&out_gpio);

    gpio_config_t in_gpio = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = GPIO_INPUT_PIN_SEL,
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&in_gpio);

    xTaskCreate(event_listener, "polls for events", 2048, NULL, 10, NULL);

    garage_state = init_garage_state();
    if (garage_state == INVALID_LIMIT_SW_S)
        return ESP_FAIL;

    return ESP_OK;
}

#endif
/* GARAGE_CONTROL_H */