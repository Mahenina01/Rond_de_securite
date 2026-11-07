package com.fortico.ronde_security.service;

import java.nio.charset.StandardCharsets;
import java.time.Instant;
import java.util.UUID;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.integration.annotation.ServiceActivator;
import org.springframework.messaging.Message;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.transaction.support.TransactionSynchronization;
import org.springframework.transaction.support.TransactionSynchronizationManager;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fortico.ronde_security.dtos.logs.LogsRequest;
import com.fortico.ronde_security.dtos.logs.LogsResponse;
import com.fortico.ronde_security.enums.LogStatus;
import com.fortico.ronde_security.model.Location;
import com.fortico.ronde_security.repo.LocationRepository;
import com.fortico.ronde_security.service.logs.LogService;

@Service
public class MqttMessageService {

  private static final Logger log = LoggerFactory.getLogger(MqttMessageService.class);

  private static final String TOPIC_PREFIX = "ronde/boitier/";
  private static final String DATA_SUFFIX = "/data";

  private final ObjectMapper objectMapper;
  private final LogService logService;
  private final LocationRepository locationRepository;
  private final MqttPublisherService mqttPublisherService;

  public MqttMessageService(
      ObjectMapper objectMapper,
      LogService logService,
      LocationRepository locationRepository,
      MqttPublisherService mqttPublisherService) {
    this.objectMapper = objectMapper;
    this.logService = logService;
    this.locationRepository = locationRepository;
    this.mqttPublisherService = mqttPublisherService;
  }

  @ServiceActivator(inputChannel = "mqttInputChannel")
  @Transactional
  public void receiveMessage(Message<String> message) {
    String topic = message.getHeaders()
        .get("mqtt_topic", String.class);

    String payload = message.getPayload();

    try {
      if (topic == null || topic.isBlank()) {
        throw new IllegalArgumentException(
            "Le topic MQTT est absent du message.");
      }

      if (payload == null || payload.isBlank()) {
        throw new IllegalArgumentException(
            "Le payload MQTT est vide.");
      }

      String deviceId = extractDeviceId(topic);

      log.info(
          "Message MQTT reçu : topic={}, device={}",
          topic, deviceId);

      JsonNode json = objectMapper.readTree(payload);

      if (!json.isObject()) {
        throw new IllegalArgumentException(
            "Le payload MQTT doit être un objet JSON.");
      }

      String checkpointCode = requiredText(json, "id_checkpoint");
      Instant timestamp = parseTimestamp(json.get("timestamp"));

      UUID eventUuid = parseEventUuid(
          json.get("id"), deviceId, payload);

      Location location = locationRepository
          .findByCheckpointCode(checkpointCode)
          .orElseThrow(() -> new IllegalArgumentException(
              "Checkpoint introuvable : " + checkpointCode));

      if (location.getAgent() == null) {
        throw new IllegalArgumentException(
            "Aucun agent associé au checkpoint : "
                + checkpointCode);
      }

      String agentCode = location.getAgent().getAgentCode();

      LogsRequest request = new LogsRequest(
          eventUuid,
          agentCode,
          deviceId,
          location.getId(),
          LogStatus.SUCCESS,
          timestamp);

      LogsResponse savedLog = logService.createLog(request);

      String ackPayload = objectMapper.writeValueAsString(
          new AckMessage(
              savedLog.id(),
              savedLog.eventUuid(),
              "ACK",
              "Log enregistré"));

      /*
       * Ne publier l'ACK positif qu'après la validation
       * de la transaction en cours.
       */
      long id = savedLog.id();
      String finalDeviceId = deviceId;
      String finalAckPayload = ackPayload;
      UUID finalEventUuid = savedLog.eventUuid();

      if (TransactionSynchronizationManager
          .isSynchronizationActive()) {

        TransactionSynchronizationManager.registerSynchronization(
            new TransactionSynchronization() {
              @Override
              public void afterCommit() {
                publishAckSafely(
                    id,
                    finalDeviceId,
                    finalAckPayload,
                    finalEventUuid);
              }
            });

      } else {
        // Si aucune synchronisation transactionnelle n'est active.
        publishAckSafely(
            id,
            finalDeviceId,
            finalAckPayload,
            finalEventUuid);
      }

    } catch (Exception exception) {
      log.error(
          "Échec du traitement MQTT : topic={}, payload={}",
          topic, payload, exception);
    }
  }

  private void publishAckSafely(
      long id,
      String deviceId,
      String ackPayload,
      UUID eventUuid) {

    try {
      mqttPublisherService.publishAck(deviceId, ackPayload);

      log.info(
          "ACK envoyé : id={}, eventUuid={}, device={}",
          id, eventUuid, deviceId);

    } catch (Exception exception) {
      log.error(
          "Échec de publication de l'ACK : eventUuid={}, device={}",
          eventUuid, deviceId, exception);
    }
  }

  private String extractDeviceId(String topic) {
    if (!topic.startsWith(TOPIC_PREFIX)
        || !topic.endsWith(DATA_SUFFIX)) {
      throw new IllegalArgumentException(
          "Topic MQTT inattendu : " + topic);
    }

    String deviceId = topic.substring(
        TOPIC_PREFIX.length(),
        topic.length() - DATA_SUFFIX.length());

    if (deviceId.isBlank() || deviceId.contains("/")) {
      throw new IllegalArgumentException(
          "Identifiant de boîtier invalide dans le topic.");
    }

    return deviceId;
  }

  private String requiredText(JsonNode json, String field) {
    JsonNode node = json.get(field);

    if (node == null || node.isNull()
        || !node.isValueNode()
        || node.asText().isBlank()) {
      throw new IllegalArgumentException(
          "Champ obligatoire absent ou invalide : " + field);
    }

    return node.asText();
  }

  private Instant parseTimestamp(JsonNode node) {
    if (node == null || node.isNull()) {
      throw new IllegalArgumentException(
          "Champ timestamp absent.");
    }

    if (node.isNumber()) {
      // Les timestamps numériques sont interprétés en millisecondes.
      return Instant.ofEpochMilli(node.asLong());
    }

    if (!node.isTextual() || node.asText().isBlank()) {
      throw new IllegalArgumentException(
          "Format du timestamp invalide.");
    }

    return Instant.parse(node.asText());
  }

  private UUID parseEventUuid(
      JsonNode logIdNode,
      String deviceId,
      String payload) {

    if (logIdNode != null && !logIdNode.isNull()
        && !logIdNode.asText().isBlank()) {

      String logId = logIdNode.asText();

      try {
        return UUID.fromString(logId);
      } catch (IllegalArgumentException ignored) {
        return UUID.nameUUIDFromBytes(
            (deviceId + ":" + logId)
                .getBytes(StandardCharsets.UTF_8));
      }
    }

    return UUID.nameUUIDFromBytes(
        (deviceId + ":" + payload)
            .getBytes(StandardCharsets.UTF_8));
  }

  public record AckMessage(
      long id,
      UUID eventUuid,
      String status,
      String message) {
  }
}
