# Soluție tema 02 — Darius Borca

Namespace MQTT: `ssatr/borca-darius/devices/<deviceId>/`.

## Implementare

Sunt create două cozi cu elemente prin valoare și un mutex pentru setări înainte de pornirea MQTT. Taskurile pornesc după mqtt_start, cu stive de 4096 octeți:

| Task | Tip / prioritate | Comportament |
|---|---|---|
| periodic | periodic / 4 | temperatureRead, telemetrie, eveniment alarmă; xTaskDelayUntil |
| sporadic | sporadic / 3 | coadă de alarme; minimum 5000 ms între sosiri acceptate |
| aperiodic | aperiodic / 2 | coadă de comenzi; validare și confirmare status |

Evenimentul alarmei include temperatura, pragul și millis la producere. Intervalele folosesc aritmetică unsigned pentru rollover. Mutexul protejează perioada și pragul, fără publicații MQTT în secțiunea protejată. Cozile pline sunt semnalate pe Serial; callback-ul nu așteaptă.

SET_INTERVAL acceptă 50..3600000 ms; SET_THRESHOLD acceptă numere finite. RESET readuce perioada la 2000 ms și pragul la 45 °C (senzorul fizic nu se resetează). STATUS arată setările. Comenzile invalide produc ERROR. Modificarea perioadei se aplică la jobul următor, fără întreruperea așteptării curente.

## Rulare

Deschideți start/esp32_device/esp32_device.ino în Arduino IDE. Alegeți ESP32C3 Dev Module, USB CDC On Boot Enabled și portul plăcii. Folosiți Arduino-ESP32 3.3.0. Configurați temporar rețeaua de 2.4 GHz în config.h; înainte de commit restabiliți iot-demo/demo123!, valorile publice ale laboratorului. Nu salvați parola personală pe GitHub.

Încărcați schița și deschideți Serial Monitor la 115200. ID-ul esp32-xxxxxx este derivat din MAC. Din start/business-service rulați:

```powershell
mvn clean test
mvn compile exec:java
```

Trimiteți comenzile folosind ID-ul afișat de placă, de exemplu esp32-a1b2c3 STATUS. Pentru alarme după stabilizarea cipului, setați pragul cu un grad sub temperatura publicată. Verificați show și fișierele data/*.csv. Serviciul are aceeași implementare ca tema 1.

## Test automat fără placă

Parserul din device_model.h este folosit atât în schiță, cât și în testul nativ:

```powershell
g++ -std=c++11 -Wall -Wextra -pedantic tema-02-esp32-mqtt/tests/device_model_test.cpp -o device_model_test.exe
./device_model_test.exe
```

Rulați din rădăcina repository-ului. Testele acoperă comenzile valide, limitele perioadei, lipsa parametrilor, parametri suplimentari, overflow, NaN/inf și RESET. Pentru validarea senzorului și a comunicației pe placă este necesar testul fizic.

Publicarea ESP32 folosește coada internă MQTT pentru ca rețeaua să nu blocheze taskul periodic ([ESP-IDF 5.5](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32c3/api-reference/protocols/mqtt.html)). Mesajele de comandă supradimensionate sau fragmentate sunt respinse, nu executate parțial.
