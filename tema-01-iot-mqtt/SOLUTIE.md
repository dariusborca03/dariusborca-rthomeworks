# Soluție tema 01 — Darius Borca

Namespace MQTT: `ssatr/borca-darius/devices/<deviceId>/`.

## Implementare

Cele trei fire sunt independente: periodic (inițial 2000 ms), sporadic (minimum 5000 ms între sosirile acceptate) și aperiodic (coadă FIFO de comenzi). Taskul periodic folosește System.nanoTime și așteptare absolută; la depășiri sare eliberările ratate, păstrând grila. Schimbarea perioadei se aplică la următorul job; o așteptare deja începută se termină la termenul planificat.

Evenimentul alarmei capturează temperatura, pragul și momentul monoton la producere. Schimbarea ulterioară a pragului nu modifică alarmele deja produse. Prima alarmă este acceptată; următoarele sosiri prea dese sunt respinse. Acesta este un simulator pe JVM, fără garanții hard real-time.

Comenzile sunt SET_INTERVAL (50..3600000 ms), SET_THRESHOLD (număr finit), RESET (doar senzorul) și STATUS. Parametrii suplimentari și comenzile invalide produc ERROR; taskul continuă. Serviciul stochează mesajele nemodificate, inclusiv mesajele invalide, și actualizează dashboard-ul doar cu date valide. Accesul concurent la starea fiecărui device este sincronizat. Timestampurile vechi nu suprascriu temperatura mai nouă.

## Rulare

Cu JDK 17+ și Maven, în două terminale, din directoarele indicate:

```powershell
cd start/business-service
mvn clean test
mvn compile exec:java
```

```powershell
cd start/device-simulator
mvn clean test
mvn compile exec:java '-Dexec.args=device-1'
```

În consola serviciului:

```text
show
device-1 SET_INTERVAL 1000
device-1 SET_THRESHOLD 20
device-1 STATUS
device-1 RESET
device-1 SET_INTERVAL 0
exit
```

CSV-urile se creează în data/ relativ la directorul din care pornește serviciul. Pentru două dispozitive porniți încă un simulator cu device-2. Diagramele cerute (surse editabile și imagini) sunt în [diagrame/](diagrame/).
