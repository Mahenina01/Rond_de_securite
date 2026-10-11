package com.fortico.ronde_security.service;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.integration.annotation.ServiceActivator;
import org.springframework.messaging.Message;
import org.springframework.stereotype.Service;

@Service
public class MqttMessageService {

  private static final Logger log = LoggerFactory.getLogger(MqttMessageService.class);

  /**
   * Reçoit les messages des boîtiers ESP32.
   */
  @ServiceActivator(inputChannel = "mqttInputChannel")
  public void receiveMessage(Message<String> message) {

    String payload = message.getPayload();

    String topic = String.valueOf(
        message.getHeaders().get("mqtt_receivedTopic"));

    log.info("Message MQTT reçu sur le topic : {}", topic);
    log.info("Contenu reçu : {}", payload);

    // TODO : Traiter le message reçu. Les étapes suivantes sont envisagées :
    // 1. Convertir le JSON en objet Java.
    // 2. Valider le device_id, le checkpoint et l'horodatage.
    // 3. Enregistrer le scan dans PostgreSQL via un service métier.
    // 4. Publier l'ACK métier seulement après la réussite
    // de l'enregistrement en base.
  }
}
