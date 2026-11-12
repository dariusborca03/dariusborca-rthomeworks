# Verificare teme 1–3 — Darius Borca

Data: 12 noiembrie 2026. Tema 4 este amânată și nu a fost modificată.

## Runda 1: implementare și teste

- Compilare și teste Maven pentru simulator, ambele servicii business, dashboard Swing și dashboard web: succes.
- Teste JUnit pentru comenzi, alarme și telemetrie; salvare CSV după mesaje invalide și actualizări concurente ale dashboard-ului: succes.
- Modelul semaforului, compilat cu g++ și testat la limitele temporale, inclusiv rollover uint32: succes.
- Toate configurațiile STUDENT din temele 1–3 și exemplul MQTT folosesc borca-darius.
- Diagramele temei 1 au surse draw.io editabile, SVG și PNG. PNG-urile au fost inspectate vizual.

## Runda 2: revizie independentă și integrare

Revizia separată a acoperit cerințele, mutexurile, ISR-ul, prioritățile și protocoalele. Problema găsită la mesajele procesate în altă ordine a fost corectată: timestampul ultimului mesaj nu regresează, iar telemetria veche nu suprascrie temperatura nouă.

- Maven clean verify pentru cele cinci aplicații și ambele proiecte de exemple ale temei 1: succes. Cele trei proiecte cu teste au câte 3 teste JUnit (9 în total), fără erori.
- Testul nativ al parserului ESP32 din tema 2: succes pentru comenzi valide/invalide, limite numerice, overflow, NaN/inf și RESET.
- Revizie și testare independentă a automatului din tema 3: succes.
- Integrare reală tema 1 prin control.aut.utcluj.ro:11188, cu simulator și serviciu ca procese separate și ID de test unic: succes.
- SET_INTERVAL 200 a produs un interval observat de 199 ms. SET_THRESHOLD -100 a declanșat alarme; două alarme observate au fost separate de 5201 ms, cu evenimente intermediare respinse. Sunt observații pe JVM, nu garanții hard real-time.
- STATUS, RESET și SET_INTERVAL 0 (ERROR, fără oprirea aplicației): succes.
- CSV telemetry/alarm/status conțin exact mesajele observate, iar show afișează dispozitivul. Procesele de test au fost oprite după verificare.

## Compilare firmware ESP32-C3

Ambele schițe au fost compilate cu Arduino-ESP32 3.3.0, USB CDC On Boot activat:

| Temă | Flash | RAM global | Rezultat |
|---|---|---|---|
| 2 | 1116826 / 1310720 bytes (85%) | 36680 / 327680 bytes (11%) | succes |
| 3 | 1098714 / 1310720 bytes (83%) | 36920 / 327680 bytes (11%) | succes |

Comenzi din rădăcina repository-ului (Arduino CLI în PATH):

```powershell
arduino-cli compile --jobs 2 --fqbn esp32:esp32:esp32c3:CDCOnBoot=cdc tema-02-esp32-mqtt/start/esp32_device
arduino-cli compile --jobs 2 --fqbn esp32:esp32:esp32c3:CDCOnBoot=cdc tema-03-semafor-simplu/start/esp32_semafor
```

## Limitele verificării

Nu a fost detectată o placă ESP32 pe port serial. Testarea fizică a temperaturii, BOOT, LED și Wi-Fi rămâne de efectuat pe placă, folosind pașii din SOLUTIE.md pentru temele 2 și 3. Testele native verifică logica folosită efectiv în firmware, nu partea electrică sau funcționarea pe hardware.

Nu sunt incluse parole personale Wi-Fi; config.h păstrează valorile publice iot-demo ale laboratorului.
