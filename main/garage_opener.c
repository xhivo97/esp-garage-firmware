#include "garage_opener.h"

static xQueueHandle gpio_evt_queue = NULL;

static enum garage_states garage_state = IDLE_S;

static TimerHandle_t light_off_timer = NULL;
static TimerHandle_t garage_open_timer = NULL;
static TimerHandle_t debounce_timer = NULL;
static TimerHandle_t rate_limit_timer = NULL;
static TimerHandle_t motor_run_limit_timer = NULL;

void update_status();
void door_open_notification();

void light_on() {
    if (light_off_timer == NULL) {
        //printf("Could not create one of the timers.\n");
    } else {
        //printf("Starting light off timer...\n");
        xTimerStart(light_off_timer, portMAX_DELAY);
    }
    gpio_set_level(LIGHT_GPIO, 1);

}
void garage_open() {
    xTimerStart(motor_run_limit_timer, portMAX_DELAY);
    light_on();
    gpio_set_level(RELAY_A_GPIO, 1);
    gpio_set_level(RELAY_B_GPIO, 0);
}
void garage_close() {
    xTimerStart(motor_run_limit_timer, portMAX_DELAY);
    light_on();
    gpio_set_level(RELAY_A_GPIO, 0);
    gpio_set_level(RELAY_B_GPIO, 1);
}
void garage_stop() {
    xTimerStop(motor_run_limit_timer, portMAX_DELAY);
    light_on();
    gpio_set_level(RELAY_A_GPIO, 0);
    gpio_set_level(RELAY_B_GPIO, 0);
}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void lightOffCallback(TimerHandle_t xTimer) {
    gpio_set_level(LIGHT_GPIO, 0);
}

void garageOpenCallback(TimerHandle_t xTimer) {
    run_state_machine();
    update_status();
    door_open_notification();
}

void debounceCallback(TimerHandle_t xTimer) {
    if(!gpio_get_level(BUTTON_GPIO)) {
        run_state_machine();
        update_status();
    }
}

void rateLimitCallback(TimerHandle_t xTimer) {
    run_state_machine();
    update_status();
}
void motorRunLimitCallback(TimerHandle_t xTimer) {
    garage_stop();
}

void queue_state_machine() {
    xTimerStart(rate_limit_timer, portMAX_DELAY);
}

static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            if(!gpio_get_level(io_num)) {
                switch(io_num) {
                    case LIMIT_A_GPIO:
                        garage_stop();
                        printf("OPENED\n");
                        garage_state = OPENED_S;
                        update_status();
                    break;
                    case LIMIT_B_GPIO:
                        garage_stop();
                        garage_state = CLOSED_S;
                        printf("CLOSED\n");
                        xTimerStop(garage_open_timer, portMAX_DELAY);
                        update_status();
                    break;
                    case BUTTON_GPIO:
                        xTimerStart(debounce_timer, portMAX_DELAY);
                    break;
                }
            }
        }
    }
}

void setup_gpios() {
    light_off_timer = xTimerCreate(
        "Light off timer",
        (LIGHT_ON_MINUTES * 60000) / portTICK_PERIOD_MS,
        pdFALSE,
        (void *)0,
        lightOffCallback
    );
    garage_open_timer = xTimerCreate(
        "Garage open timer",
        (GARAGE_OPEN_ALERT_MINUTES * 60000) / portTICK_PERIOD_MS,
        pdFALSE,
        (void *)1,
        garageOpenCallback
    );
    debounce_timer = xTimerCreate(
        "Debounce open timer",
        100 / portTICK_PERIOD_MS,
        pdFALSE,
        (void *)2,
        debounceCallback
    );
    rate_limit_timer = xTimerCreate(
        "Rate limit timer",
        500 / portTICK_PERIOD_MS,
        pdFALSE,
        (void *)3,
        rateLimitCallback
    );
    motor_run_limit_timer = xTimerCreate(
        "Motor run limit timer",
        (MOTOR_RUN_LIMIT_SECONDS * 1000) / portTICK_PERIOD_MS,
        pdFALSE,
        (void *)4,
        motorRunLimitCallback
    );
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(LIMIT_A_GPIO, gpio_isr_handler, (void*) LIMIT_A_GPIO);
    gpio_isr_handler_add(LIMIT_B_GPIO, gpio_isr_handler, (void*) LIMIT_B_GPIO);
    gpio_isr_handler_add(BUTTON_GPIO, gpio_isr_handler, (void*) BUTTON_GPIO);

    if (!gpio_get_level(LIMIT_A_GPIO)) {
        gpio_set_level(RELAY_A_GPIO, 0);
        gpio_set_level(RELAY_B_GPIO, 0);
        printf("OPENED\n");
        garage_state = OPENED_S;
    }
    if (!gpio_get_level(LIMIT_B_GPIO)) {
        gpio_set_level(RELAY_A_GPIO, 0);
        gpio_set_level(RELAY_B_GPIO, 0);
        printf("CLOSED\n");
        garage_state = CLOSED_S;
    }
}

void run_state_machine() {
    xTimerStart(garage_open_timer, portMAX_DELAY);
    switch (garage_state) {
        case OPENED_S:
            garage_close();
            garage_state = CLOSING_S;
        break;
        case CLOSED_S:
            garage_open();
            garage_state = OPENING_S;
        break;
        case OPENING_S:
            garage_stop();
            garage_state = STOPPED_WHILE_OPENING_S;
        break;
        case CLOSING_S:
            garage_stop();
            garage_state = STOPPED_WHILE_CLOSING_S;
        break;
        case STOPPED_WHILE_OPENING_S:
            garage_close();
            garage_state = CLOSING_S;
        break;
        case STOPPED_WHILE_CLOSING_S:
            garage_open();
            garage_state = OPENING_S;
        break;
        case FLOATING_S:
            garage_open();
            garage_state = OPENING_S;
        break;
        case IDLE_S:
            garage_open();
            garage_state = OPENING_S;
        break;
        case LIMIT_SWITCH_ERROR_S:
        break;
    }
}

char* get_garage_state() {
    char* r = "";
    switch (garage_state) {
        case OPENED_S:
            r = "OPENED";
        break;
        case CLOSED_S:
            r = "CLOSED";
        break;
        case OPENING_S:
            r = "OPENING";
        break;
        case CLOSING_S:
            r = "CLOSING";
        break;
        case STOPPED_WHILE_OPENING_S:
            r = "STOPPED WHILE OPENING";
        break;
        case STOPPED_WHILE_CLOSING_S:
            r = "STOPPED WHILE CLOSING";
        break;
        case FLOATING_S:
            r = "FLOATING";
        break;
        case IDLE_S:
            r = "IDLE";
        break;
        case LIMIT_SWITCH_ERROR_S:
            r = "ERROR";
        break;
    }
    return r;
}