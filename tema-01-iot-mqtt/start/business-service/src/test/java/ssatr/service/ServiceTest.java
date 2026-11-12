package ssatr.service;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;
import static org.junit.jupiter.api.Assertions.*;
import java.nio.file.*;
import java.util.concurrent.*;
import java.io.*;
class ServiceTest {
 @TempDir Path directory;
 @Test void malformedMessagesDoNotStopWorkerAndCsvPreservesPayload() throws Exception {
    var inbox = new LinkedBlockingQueue<String>(); var dashboard = new Dashboard();
    var worker = new Thread(new MessageProcessor("telemetry", inbox, new CsvStorage(directory.toString()), dashboard));
    worker.start();
    try {
      inbox.add("broken"); inbox.add("bad;10;NaN"); inbox.add("device-1;100;23.45");
      Path file = directory.resolve("telemetry.csv");
      long deadline = System.nanoTime() + TimeUnit.SECONDS.toNanos(3);
      while ((!Files.exists(file) || !Files.readString(file).contains("device-1;100;23.45")) && System.nanoTime()<deadline) Thread.sleep(10);
      assertTrue(Files.readString(file).contains("broken\nbad;10;NaN\ndevice-1;100;23.45\n"));
      assertTrue(worker.isAlive());
    } finally { worker.interrupt(); worker.join(2000); }
 }
 @Test void dashboardKeepsEveryConcurrentAlarm() throws Exception {
    Dashboard d = new Dashboard(); var pool = Executors.newFixedThreadPool(4);
    for(int i=0;i<1000;i++) pool.submit(()->d.updateAlarm("device-1",100));
    pool.shutdown(); assertTrue(pool.awaitTermination(3,TimeUnit.SECONDS));
    var original = System.out; var output = new ByteArrayOutputStream();
    try { System.setOut(new PrintStream(output)); d.print(); } finally { System.setOut(original); }
    assertTrue(output.toString().contains("1000"));
 }

 @Test void olderTelemetryCannotReplaceNewerTemperature() {
    Dashboard d = new Dashboard();
    d.updateTelemetry("device-1",42.5,200);
    d.updateTelemetry("device-1",10,100);
    d.updateStatus("device-1",50);
    d.updateAlarm("device-1",75);
    var original = System.out; var output = new ByteArrayOutputStream();
    try { System.setOut(new PrintStream(output)); d.print(); } finally { System.setOut(original); }
    assertTrue(output.toString().matches("(?s).*42[.,]50.*"));
 }
}
