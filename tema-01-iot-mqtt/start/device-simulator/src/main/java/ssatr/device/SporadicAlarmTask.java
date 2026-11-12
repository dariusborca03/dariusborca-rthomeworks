package ssatr.device;

import java.util.Locale;
import java.util.concurrent.BlockingQueue;

/**
 * TASK SPORADIC: tratează alarmele de temperatură.
 *
 * Evenimentele sosesc în coada alarmEvents (puse de taskul periodic).
 * Intervalul minim între sosiri este MIN_INTERARRIVAL_MS: între două eliberări
 * ale taskului (două alarme publicate) trebuie să treacă cel puțin atât.
 * Intervalul este impus de cod: evenimentele care sosesc mai des sunt
 * respinse, ca să nu fie inundat brokerul.
 */
public class SporadicAlarmTask implements Runnable {

    static final long MIN_INTERARRIVAL_MS = 5000;

    private final String deviceId;
    private final MqttConnection mqtt;
    private final BlockingQueue<AlarmEvent> alarmEvents;

    public SporadicAlarmTask(String deviceId, MqttConnection mqtt, BlockingQueue<AlarmEvent> alarmEvents, PeriodicTelemetryTask periodic) {
        this.deviceId = deviceId;
        this.mqtt = mqtt;
        this.alarmEvents = alarmEvents;
    }

    @Override
    public void run() {
        long lastReleaseTime = 0;
        boolean accepted = false;

        try {
            while (true) {
                // Task-ul adoarme așteptând un eveniment (blocant)
                AlarmEvent event = alarmEvents.take();
                double temp = event.temperature();
                long currentTime = System.currentTimeMillis();

                // Verificăm dacă a trecut intervalul minim de protecție împotriva inundării brokerului
                if (accepted && event.arrivalNanos() - lastReleaseTime < MIN_INTERARRIVAL_MS * 1_000_000L) {
                    System.out.println("Alarma respinsa (inundare): " + temp);
                    continue; // respingem și ne întoarcem la așteptat
                }

                lastReleaseTime = event.arrivalNanos();
                accepted = true;

                // Folosim pragul capturat la sosire, nu setarea posibil schimbata intre timp.
                double currentThreshold = event.threshold();

                // Publicam alarma cu temperatura si pragul aceluiasi eveniment.
                String payload = String.format(Locale.US, "%s;%d;%.2f;%.2f", deviceId, currentTime, temp, currentThreshold);
                mqtt.publish(Config.alarmTopic(deviceId), payload);
                System.out.println("[ALARM START] Trimis: " + payload);
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            System.out.println("SporadicAlarmTask a fost intrerupt.");
        }
    }
}
