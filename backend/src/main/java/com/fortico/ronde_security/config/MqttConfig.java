
package com.fortico.ronde_security.config;

import java.nio.charset.StandardCharsets;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.hivemq.client.mqtt.MqttClient;
import com.hivemq.client.mqtt.MqttClientState;
import com.hivemq.client.mqtt.datatypes.MqttQos;
import com.hivemq.client.mqtt.mqtt5.Mqtt5BlockingClient;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.ApplicationRunner;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.integration.annotation.ServiceActivator;
import org.springframework.integration.channel.DirectChannel;
import org.springframework.messaging.Message;
import org.springframework.messaging.MessageChannel;
import org.springframework.messaging.MessageHandler;
import org.springframework.messaging.support.MessageBuilder;

@Configuration
public class MqttConfig {

  @Value("${mqtt.broker-host}")
  private String brokerHost;

  @Value("${mqtt.broker-port}")
  private int brokerPort;

  @Value("${mqtt.client-id}")
  private String clientId;

  @Value("${mqtt.username}")
  private String username;

  @Value("${mqtt.password}")
  private String password;

  @Value("${mqtt.data-topic}")
  private String dataTopic;

  @Value("${mqtt.tls-enabled:false}")
  private boolean tlsEnabled;

  @Bean
  public Mqtt5BlockingClient mqttClient() {
    var builder = MqttClient.builder()
        .useMqttVersion5()
        .identifier(clientId)
        .serverHost(brokerHost)
        .serverPort(brokerPort)
        .automaticReconnectWithDefaultConfig();

    if (tlsEnabled) {
      builder = builder.sslWithDefaultConfig();
    }

    return builder
        .simpleAuth()
        .username(username)
        .password(password.getBytes(StandardCharsets.UTF_8))
        .applySimpleAuth()
        .buildBlocking();
  }

  /**
   * Canal de réception des messages MQTT.
   */
  @Bean
  public MessageChannel mqttInputChannel() {
    return new DirectChannel();
  }

  /**
   * Canal de publication vers les boîtiers.
   */
  @Bean
  public MessageChannel mqttOutboundChannel() {
    return new DirectChannel();
  }

  /**
   * Connexion au broker et abonnement aux messages des boîtiers.
   */
  @Bean
  public ApplicationRunner mqttConnectionStarter(
      Mqtt5BlockingClient mqttClient,
      MessageChannel mqttInputChannel) {

    return args -> {
      mqttClient.connect();

      System.out.println(
          "Connexion HiveMQ établie : " + brokerHost);

      mqttClient.toAsync().subscribeWith()
          .topicFilter(dataTopic)
          .qos(MqttQos.AT_LEAST_ONCE)
          .callback(publish -> {
            String payload = new String(
                publish.getPayloadAsBytes(),
                StandardCharsets.UTF_8);

            String topic = publish.getTopic().toString();

            System.out.println(
                "Message MQTT reçu sur " + topic
                    + " : " + payload);

            Message<String> message = MessageBuilder
                .withPayload(payload)
                .setHeader("mqtt_topic", topic)
                .setHeader("mqtt_qos", publish.getQos())
                .build();

            mqttInputChannel.send(message);
          })
          .send()
          .whenComplete((subAck, error) -> {
            if (error != null) {
              System.err.println(
                  "Erreur d'abonnement MQTT : "
                      + error.getMessage());
            } else {
              System.out.println(
                  "Abonnement MQTT actif : " + dataTopic);
            }
          });
    };
  }

  /**
   * Publication des messages sortants.
   */
  @Bean
  @ServiceActivator(inputChannel = "mqttOutboundChannel")
  public MessageHandler mqttOutbound(
      Mqtt5BlockingClient mqttClient) {

    return message -> {
      if (mqttClient.getState() != MqttClientState.CONNECTED) {
        throw new IllegalStateException(
            "Le client HiveMQ n'est pas connecté.");
      }

      Object payload = message.getPayload();

      String topic = message.getHeaders()
          .get("mqtt_topic", String.class);

      if (topic == null || topic.isBlank()) {
        topic = message.getHeaders()
            .get("mqtt_topicName", String.class);
      }

      if (topic == null || topic.isBlank()) {
        throw new IllegalArgumentException(
            "Le topic MQTT doit être spécifié dans l'en-tête mqtt_topic.");
      }

      mqttClient.publishWith()
          .topic(topic)
          .qos(MqttQos.AT_LEAST_ONCE)
          .payload(payload.toString()
              .getBytes(StandardCharsets.UTF_8))
          .send();
    };
  }

  @Bean
  public ObjectMapper objectMapper() {
    return new ObjectMapper().findAndRegisterModules();
  }
}
