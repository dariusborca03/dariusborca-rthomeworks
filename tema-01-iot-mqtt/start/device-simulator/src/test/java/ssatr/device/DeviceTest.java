package ssatr.device;
import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;
import java.util.concurrent.*;
import java.util.*;

class DeviceTest {
    static class FakeMqtt extends MqttConnection {
        final BlockingQueue<String> published = new LinkedBlockingQueue<>();
        FakeMqtt() throws Exception { super("test"); }
        @Override public void publish(String topic, String text) { published.add(text); }
    }
    @Test void commandsRejectInvalidValuesAndContinue() throws Exception {
        FakeMqtt mqtt = new FakeMqtt(); TemperatureSensor sensor = new TemperatureSensor();
        var alarms = new LinkedBlockingQueue<AlarmEvent>(); var commands = new LinkedBlockingQueue<String>();
        var periodic = new PeriodicTelemetryTask("test", sensor, mqtt, alarms);
        Thread worker = new Thread(new AperiodicCommandTask("test", sensor, mqtt, commands, periodic));
        worker.start();
        try {
            for (String c : List.of("SET_INTERVAL 0", "SET_INTERVAL -1", "SET_INTERVAL 9999999999", "SET_THRESHOLD NaN", "SET_THRESHOLD Infinity", "RESET extra", "STATUS extra", "SET_INTERVAL 100 extra", "UNKNOWN", "BAD;COMMAND", "")) {
                commands.add(c); String reply = mqtt.published.poll(2, TimeUnit.SECONDS);
                assertTrue(reply.contains(";ERROR"), c); assertEquals(3, reply.split(";", -1).length);
            }
            commands.add("SET_INTERVAL 100"); assertTrue(mqtt.published.poll(2, TimeUnit.SECONDS).contains(";OK"));
            commands.add("SET_THRESHOLD 21.5"); assertTrue(mqtt.published.poll(2, TimeUnit.SECONDS).contains(";OK"));
            commands.add("STATUS"); assertTrue(mqtt.published.poll(2, TimeUnit.SECONDS).contains("interval=100 threshold=21.5"));
            commands.add("RESET"); assertTrue(mqtt.published.poll(2, TimeUnit.SECONDS).contains(";OK RESET"));
            assertEquals(100, periodic.getPeriodMs()); // Tema 01: RESET reseteaza senzorul, nu configuratia.
        } finally { worker.interrupt(); worker.join(2000); mqtt.disconnect(); }
        assertFalse(worker.isAlive());
    }
    @Test void sporadicUsesArrivalAndCapturedThreshold() throws Exception {
        FakeMqtt mqtt = new FakeMqtt(); var queue = new LinkedBlockingQueue<AlarmEvent>();
        var periodic = new PeriodicTelemetryTask("test", new TemperatureSensor(), mqtt, queue);
        Thread worker = new Thread(new SporadicAlarmTask("test", mqtt, queue, periodic));
        queue.add(new AlarmEvent(30, 28, 1));
        queue.add(new AlarmEvent(31, 28, 1_000_000_001L)); // reject, even if processed later
        queue.add(new AlarmEvent(32, 29, 5_000_000_001L));
        worker.start();
        try {
            assertTrue(mqtt.published.poll(2, TimeUnit.SECONDS).endsWith(";30.00;28.00"));
            assertTrue(mqtt.published.poll(2, TimeUnit.SECONDS).endsWith(";32.00;29.00"));
            assertNull(mqtt.published.poll(150, TimeUnit.MILLISECONDS));
        } finally { worker.interrupt(); worker.join(2000); mqtt.disconnect(); }
    }
    @Test void periodicPublishesAndProducesAlarmEvents() throws Exception {
        FakeMqtt mqtt = new FakeMqtt(); var queue = new LinkedBlockingQueue<AlarmEvent>();
        var periodic = new PeriodicTelemetryTask("test", new TemperatureSensor(), mqtt, queue);
        periodic.setPeriodMs(50); periodic.setThreshold(-100);
        Thread worker = new Thread(periodic); worker.start();
        try {
            for (int i=0;i<4;i++) {
                String[] fields = mqtt.published.poll(2, TimeUnit.SECONDS).split(";");
                assertEquals(3, fields.length); assertEquals("test", fields[0]);
                assertTrue(Long.parseLong(fields[1]) > 0); assertTrue(Double.isFinite(Double.parseDouble(fields[2])));
                assertEquals(-100, queue.poll(2, TimeUnit.SECONDS).threshold());
            }
        } finally { worker.interrupt(); worker.join(2000); mqtt.disconnect(); }
    }
}
