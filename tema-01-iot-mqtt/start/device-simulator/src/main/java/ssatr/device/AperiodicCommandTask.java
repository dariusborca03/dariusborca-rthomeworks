package ssatr.device;

import java.util.concurrent.BlockingQueue;

/**
 * TASK APERIODIC: execută comenzile primite de la business service.
 *
 * Comenzile sosesc oricând, pe MQTT, fără un interval minim între ele, și sunt
 * puse în coada commands. Fiecare comandă eliberează un job al taskului.
 * Comenzi care trebuie suportate:
 *
 *   SET_INTERVAL <ms>       schimbă perioada T a taskului periodic
 *   SET_THRESHOLD <valoare> schimbă pragul de alarmă
 *   RESET                   readuce senzorul la valoarea inițială
 *   STATUS                  raportează perioada și pragul curente
 *
 * După executarea fiecărei comenzi, device-ul publică pe Config.statusTopic(deviceId)
 * o confirmare: "deviceId;timestamp;text", de exemplu
 * "device-1;1727600000000;OK SET_INTERVAL 1000".
 */
public class AperiodicCommandTask implements Runnable {

    private final String deviceId;
    private final TemperatureSensor sensor;
    private final MqttConnection mqtt;
    private final BlockingQueue<String> commands;
    private final PeriodicTelemetryTask periodic;

    public AperiodicCommandTask(String deviceId, TemperatureSensor sensor, MqttConnection mqtt,
                                BlockingQueue<String> commands, PeriodicTelemetryTask periodic) {
        this.deviceId = deviceId;
        this.sensor = sensor;
        this.mqtt = mqtt;
        this.commands = commands;
        this.periodic = periodic;
    }

    @Override
    public void run() {
        try {
            while (true) {
                // Așteaptă asincron sosirea unei comenzi
                String commandStr = commands.take();
                // Elimină eventualele spații în plus din stânga/dreapta și le desparte
                String[] parts = commandStr.trim().split("\\s+");
                String action = parts[0].toUpperCase(java.util.Locale.ROOT);
                String normalized = String.join(" ", parts);
                String responseText;

                try {
                    switch (action) {
                        case "SET_INTERVAL":
                            if (parts.length != 2) throw new IllegalArgumentException("Lipseste parametrul ms");
                            long interval = Long.parseLong(parts[1]);
                            periodic.setPeriodMs(interval);
                            responseText = "OK " + normalized;
                            break;
                        case "SET_THRESHOLD":
                            if (parts.length != 2) throw new IllegalArgumentException("Lipseste valoarea");
                            double threshold = Double.parseDouble(parts[1]);
                            periodic.setThreshold(threshold);
                            responseText = "OK " + normalized;
                            break;
                        case "RESET":
                            if (parts.length != 1) throw new IllegalArgumentException();
                            sensor.reset();
                            responseText = "OK " + normalized;
                            break;
                        case "STATUS":
                            if (parts.length != 1) throw new IllegalArgumentException();
                            responseText = "STATUS interval=" + periodic.getPeriodMs() + " threshold=" + periodic.getThreshold();
                            break;
                        default:
                            responseText = "ERROR Comanda necunoscuta";
                            break;
                    }
                } catch (Exception e) {
                    responseText = "ERROR Parametru invalid pentru " + action;
                }

                // Generăm timestamp și publicăm pe topicul de confirmări
                long timestamp = System.currentTimeMillis();
                String statusPayload = deviceId + ";" + timestamp + ";" + responseText;

                mqtt.publish(Config.statusTopic(deviceId), statusPayload);
                System.out.println("[COMMAND STATUS] " + statusPayload);
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            System.out.println("AperiodicCommandTask a fost intrerupt.");
        }
    }
}
