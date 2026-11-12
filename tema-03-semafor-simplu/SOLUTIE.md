# Soluție tema 03 — Darius Borca

Namespace MQTT: `ssatr/borca-darius/devices/<deviceId>/`.

## Implementare

Automatul începe în CAR_GREEN și folosește exclusiv millis pentru durate. Ordinea este CAR_GREEN (minimum 10 s și cerere), CAR_YELLOW (3 s), ALL_RED (2 s), PED_GREEN (8 s). La intrarea în PED_GREEN se șterge cererea; o apăsare în timpul acestui verde se păstrează pentru ciclul următor.

| Task | Perioadă / prioritate | Rol |
|---|---|---|
| control | 100 ms / 4 | tranzițiile automatului sub mutex |
| output | 500 ms / 3 | snapshot, lămpi, ceil pentru secunde, lights MQTT, LED |
| button | sporadic / 2 | minimum 1000 ms între sosiri acceptate |

Prioritățile periodice respectă Rate Monotonic. Toate stivele au 4096 octeți. ISR-ul BOOT (GPIO9, FALLING) copiază tick-ul sosirii în coadă și cere replanificarea dacă este necesar; nu folosește Serial, MQTT sau mutex. Comanda BUTTON intră în aceeași coadă. Mutexul stării nu este ținut în timpul publicării ori afișării. LED-ul GPIO8 este activ LOW și reprezintă lampa AȘTEPTAȚI.

## Rulare

Arduino-ESP32 3.3.0, ESP32C3 Dev Module, USB CDC On Boot Enabled, Serial Monitor 115200. Deschideți start/esp32_semafor/esp32_semafor.ino și încărcați pe placă. Regula Wi-Fi este aceeași ca la tema 2: păstrați doar datele publice iot-demo în commit.

Porniți unul dintre dashboard-uri:

```powershell
cd start/dashboard
mvn clean test
mvn compile exec:java
```

Alternativ, din start/dashboard-web rulați mvn compile exec:java și deschideți adresa afișată de server. STUDENT este borca-darius în ambele configurații.

## Test automat fără placă

Din rădăcina repository-ului, cu g++:

```powershell
g++ -std=c++11 -Wall -Wextra -pedantic tema-03-semafor-simplu/tests/traffic_model_test.cpp -o traffic_model_test.exe
./traffic_model_test.exe
```

Se testează modelul utilizat efectiv de schiță: lipsa cererii, limitele temporale, toate tranzițiile, numărătoarea, cererea din PED_GREEN și rollover. Testul nu înlocuiește verificarea electrică BOOT/LED sau testarea Wi-Fi pe placa reală.

Publicarea ESP32 folosește coada internă MQTT pentru ca rețeaua să nu blocheze taskul periodic ([ESP-IDF 5.5](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32c3/api-reference/protocols/mqtt.html)). Mesajele de comandă supradimensionate sau fragmentate sunt respinse, nu executate parțial.
