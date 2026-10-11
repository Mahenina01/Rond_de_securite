package com.fortico.ronde_security.service;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.integration.mqtt.support.MqttHeaders;
import org.springframework.messaging.Message;
import org.springframework.messaging.MessageChannel;
import org.springframework.messaging.support.MessageBuilder;
import org.springframework.stereotype.Service;

@Service
public class MqttPublisherService {

  private final MessageChannel mqttOutboundChannel;

  @Value("${mqtt.ack-topic-prefix}")
  private String ackTopicPrefix;

  public MqttPublisherService(
      MessageChannel mqttOutboundChannel) {
    this.mqttOutboundChannel = mqttOutboundChannel;
  }

  /**
   * Publie un accusé de réception vers un boîtier.
   *
   * @param deviceId identifiant du boîtier, par exemple PN-01
   * @param payload  contenu JSON de la réponse
   */
  public void publishAck(String deviceId, String payload) {

    if (deviceId == null || deviceId.isBlank()) {
      throw new IllegalArgumentException(
          "L'identifiant du boîtier est obligatoire.");
    }

    String topic = ackTopicPrefix + deviceId + "/ack";

    Message<String> message = MessageBuilder
        .withPayload(payload)
        .setHeader(MqttHeaders.TOPIC, topic)
        .setHeader(MqttHeaders.QOS, 1)
        .build();

    boolean sent = mqttOutboundChannel.send(message);

    if (!sent) {
      throw new IllegalStateException(
          "Échec de l'envoi du message MQTT.");
    }
  }
}