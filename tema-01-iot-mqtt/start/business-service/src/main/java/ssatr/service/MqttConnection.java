package ssatr.service;

import java.nio.charset.StandardCharsets;
import java.util.UUID;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.BlockingQueue;
import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;
/** Darius Borca: callback scurt, UTF-8 si reabonare dupa reconectare. */
public class MqttConnection {
    private final MqttClient client;
    private final Map<String, BlockingQueue<String>> subscriptions = new ConcurrentHashMap<>();
    public MqttConnection(String name) throws MqttException {
        client = new MqttClient(Config.BROKER, name + "-" + UUID.randomUUID(), new MemoryPersistence());
        client.setCallback(new MqttCallbackExtended() {
            public void connectComplete(boolean reconnect, String uri) {
                if (reconnect) subscriptions.forEach((topic, inbox) -> {
                    try { subscribe(topic, inbox); }
                    catch (MqttException e) { System.err.println("Reabonare esuata: " + topic); }
                });
            }
            public void connectionLost(Throwable cause) { System.err.println("MQTT deconectat; reincercare automata"); }
            public void messageArrived(String topic, MqttMessage message) {}
            public void deliveryComplete(IMqttDeliveryToken token) {}
        });
    }
    public void connect() throws MqttException {
        MqttConnectOptions options = new MqttConnectOptions();
        options.setCleanSession(true); options.setAutomaticReconnect(true); options.setConnectionTimeout(10);
        client.setTimeToWait(3000);
        client.connect(options);
        System.out.println("Conectat la " + Config.BROKER);
    }
    public void publish(String topic, String text) {
        try {
            MqttMessage message = new MqttMessage(text.getBytes(StandardCharsets.UTF_8));
            message.setQos(1); client.publish(topic, message);
        } catch (MqttException e) { System.err.println("Publicare esuata: " + topic + " / " + e.getMessage()); }
    }
    public void subscribe(String topic, BlockingQueue<String> inbox) throws MqttException {
        subscriptions.put(topic, inbox);
        client.subscribe(topic, 1, (receivedTopic, message) -> {
            if (!inbox.offer(new String(message.getPayload(), StandardCharsets.UTF_8)))
                System.err.println("Coada plina: " + receivedTopic);
        });
    }
    public void disconnect() throws MqttException {
        if (client.isConnected()) client.disconnect();
        client.close();
    }
}
