# Diagrame — Darius Borca

Fiecare diagramă are o sursă draw.io editabilă (`.drawio`, XML necomprimat), un export vectorial (`.svg`) și un export PNG (`.png`). Deschideți fișierele draw.io în diagrams.net.

- [Componente](componente.png): simulator, taskuri, cozi, broker și serviciu business.
- [Secvența alarmei](secventa-alarma.png): telemetrie peste prag și prima alarmă acceptată.
- [Secvența comenzii](secventa-comanda.png): `SET_INTERVAL 1000`, execuție aperiodică și confirmare.
- [Timp](timp.png): scenariu ilustrativ de 10 secunde, cu eliberări periodice, alarme acceptate/respinse și comanda `STATUS`.

Diagrama de timp este un model explicativ, nu un rezultat măsurat. Duratele joburilor nu sunt la scară; perioadele și intervalul minim dintre evenimente sunt la scară. În secvența comenzii, coloana `Business service` include `CommandConsole` pentru trimitere și `MessageProcessor` pentru confirmare.

`generate_diagrams.py` regenerează sursele și exporturile din același model geometric. Necesită Python 3, Pillow și fontul Arial din Windows. Rulare: `python generate_diagrams.py`.
