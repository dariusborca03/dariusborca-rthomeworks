// Autor: Darius Borca
#include "config.h"
#include "connection.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "traffic_model.h"
TrafficState state{CAR_GREEN, 0, false};
SemaphoreHandle_t stateMutex;
QueueHandle_t buttons;
void IRAM_ATTR button_isr() {
    TickType_t arrival = xTaskGetTickCountFromISR();
    BaseType_t wake = pdFALSE;
    xQueueSendFromISR(buttons, &arrival, &wake);
    if (wake) portYIELD_FROM_ISR();
}
void command_received(const char* text) {
    if (strcmp(text, "BUTTON") != 0) return;
    TickType_t arrival = xTaskGetTickCount();
    if (xQueueSend(buttons, &arrival, 0) != pdTRUE) Serial.println("[button] coada plina");
}
void control_task(void*) {
    TickType_t last = xTaskGetTickCount();
    for (;;) {
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        Phase old = state.phase; advance(state, millis()); Phase current = state.phase;
        xSemaphoreGive(stateMutex);
        if (old != current) Serial.printf("[control] %s\n", PHASE_NAMES[current]);
        xTaskDelayUntil(&last, pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    }
}
void output_task(void*) {
    TickType_t last = xTaskGetTickCount();
    for (;;) {
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        TrafficState snapshot = state;
        xSemaphoreGive(stateMutex);
        const char* car = snapshot.phase == CAR_GREEN ? "GREEN" : snapshot.phase == CAR_YELLOW ? "YELLOW" : "RED";
        const char* ped = snapshot.phase == PED_GREEN ? "GREEN" : "RED";
        char text[160];
        snprintf(text, sizeof(text), "%s;%lld;%s;%s;%s;%d;%u", DEVICE_ID, (long long)epoch_ms(),
            PHASE_NAMES[snapshot.phase], car, ped, snapshot.requested, seconds_left(snapshot, millis()));
        digitalWrite(LED_PIN, snapshot.requested ? LOW : HIGH);
        mqtt_publish("lights", text);
        xTaskDelayUntil(&last, pdMS_TO_TICKS(OUTPUT_PERIOD_MS));
    }
}
void button_task(void*) {
    TickType_t arrival, last = 0; bool accepted = false;
    for (;;) {
        xQueueReceive(buttons, &arrival, portMAX_DELAY);
        if (accepted && TickType_t(arrival - last) < pdMS_TO_TICKS(MIN_INTERARRIVAL_MS)) {
            Serial.println("[button] respins"); continue;
        }
        accepted = true; last = arrival;
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        state.requested = true;
        xSemaphoreGive(stateMutex);
        Serial.println("[button] cerere acceptata");
    }
}
void setup() {
    Serial.begin(115200); Serial.setTxTimeoutMs(0);
    uint32_t serialStart = millis();
    while (!Serial && uint32_t(millis() - serialStart) < 3000) delay(10);
    Serial.printf("Darius Borca - tema 03 - %s\n", DEVICE_ID);
    pinMode(LED_PIN, OUTPUT); digitalWrite(LED_PIN, HIGH);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    wifi_connect(); time_sync();
    stateMutex = xSemaphoreCreateMutex(); buttons = xQueueCreate(16, sizeof(TickType_t));
    configASSERT(stateMutex && buttons);
    state.since = millis();
    mqtt_start();
    // Rate Monotonic: control 100 ms -> prioritate 4; iesire 500 ms -> 3.
    configASSERT(xTaskCreate(control_task, "control", 4096, nullptr, 4, nullptr) == pdPASS);
    configASSERT(xTaskCreate(output_task, "output", 4096, nullptr, 3, nullptr) == pdPASS);
    configASSERT(xTaskCreate(button_task, "button", 4096, nullptr, 2, nullptr) == pdPASS);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), button_isr, FALLING);
}
void loop() { delay(1000); }
