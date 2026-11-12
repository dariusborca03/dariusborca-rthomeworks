// Autor: Darius Borca. Trei taskuri FreeRTOS, protocol compatibil tema 01.
#include "config.h"
#include "connection.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <math.h>
#include <errno.h>
#include "device_model.h"
struct AlarmEvent { float temperature, threshold; uint32_t arrival; };
struct Command { char text[COMMAND_MAX_LEN]; };
QueueHandle_t alarms, commands;
SemaphoreHandle_t settings;
uint32_t periodMs = PERIOD_MS_INITIAL;
float threshold = THRESHOLD_INITIAL;

void periodic_task(void*) {
    TickType_t last = xTaskGetTickCount();
    for (;;) {
        xSemaphoreTake(settings, portMAX_DELAY);
        uint32_t period = periodMs; float limit = threshold;
        xSemaphoreGive(settings);
        float temperature = temperatureRead();
        char text[128];
        snprintf(text, sizeof(text), "%s;%lld;%.2f", DEVICE_ID, (long long)epoch_ms(), temperature);
        mqtt_publish("telemetry", text);
        Serial.printf("[periodic] %s T=%lu\n", text, (unsigned long)period);
        if (temperature > limit) {
            AlarmEvent event{temperature, limit, millis()};
            if (xQueueSend(alarms, &event, 0) != pdTRUE) Serial.println("[alarm] coada plina");
        }
        xTaskDelayUntil(&last, pdMS_TO_TICKS(period));
    }
}
void sporadic_task(void*) {
    AlarmEvent event; uint32_t last = 0; bool accepted = false;
    for (;;) {
        xQueueReceive(alarms, &event, portMAX_DELAY);
        if (accepted && uint32_t(event.arrival - last) < MIN_INTERARRIVAL_MS) {
            Serial.println("[sporadic] alarma respinsa"); continue;
        }
        accepted = true; last = event.arrival;
        char text[128];
        snprintf(text, sizeof(text), "%s;%lld;%.2f;%.2f", DEVICE_ID, (long long)epoch_ms(), event.temperature, event.threshold);
        mqtt_publish("alarm", text); Serial.printf("[sporadic] %s\n", text);
    }
}
void command_received(const char* text) {
    Command command{};
    if (strlen(text) >= sizeof(command.text)) return;
    strlcpy(command.text, text, sizeof(command.text));
    if (xQueueSend(commands, &command, 0) != pdTRUE) Serial.println("[commands] coada plina");
}
void aperiodic_task(void*) {
    Command command;
    for (;;) {
        xQueueReceive(commands, &command, portMAX_DELAY);
        char reply[128];
        xSemaphoreTake(settings, portMAX_DELAY);
        execute_command(command.text, periodMs, threshold, reply, sizeof(reply));
        xSemaphoreGive(settings);
        char text[192];
        snprintf(text, sizeof(text), "%s;%lld;%s", DEVICE_ID, (long long)epoch_ms(), reply);
        mqtt_publish("status", text); Serial.printf("[aperiodic] %s\n", text);
    }
}
void setup() {
    Serial.begin(115200); Serial.setTxTimeoutMs(0);
    uint32_t serialStart = millis();
    while (!Serial && uint32_t(millis() - serialStart) < 3000) delay(10);
    Serial.printf("Darius Borca - tema 02 - %s\n", DEVICE_ID);
    wifi_connect(); time_sync();
    alarms = xQueueCreate(16, sizeof(AlarmEvent));
    commands = xQueueCreate(16, sizeof(Command));
    settings = xSemaphoreCreateMutex();
    configASSERT(alarms && commands && settings);
    mqtt_start();
    configASSERT(xTaskCreate(periodic_task, "periodic", 4096, nullptr, 4, nullptr) == pdPASS);
    configASSERT(xTaskCreate(sporadic_task, "sporadic", 4096, nullptr, 3, nullptr) == pdPASS);
    configASSERT(xTaskCreate(aperiodic_task, "aperiodic", 4096, nullptr, 2, nullptr) == pdPASS);
}
void loop() { delay(1000); }
