
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

    String topic = String.valueOf(
        message.getHeaders().get("mqtt_receivedTopic"));

    String payload = message.getPayload();

    String deviceId = extractDeviceId(topic);

    log.info("Message MQTT reçu : topic={}, device={}",
        topic, deviceId);

    try {
      JsonNode json = objectMapper.readTree(payload);

      String checkpointCode = requiredText(json, "tag");
      Instant timestamp = parseTimestamp(json.get("timestamp"));

      UUID eventUuid = parseEventUuid(
          json.get("log_id"), deviceId, payload);

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

      // Enregistrement via le service métier existant.
      LogsResponse savedLog = logService.createLog(request);

      // ACK uniquement après le retour réussi du service.
      String ackPayload = objectMapper.writeValueAsString(
          new AckMessage(
              savedLog.eventUuid(),
              "ACK",
              "Log enregistré"));

      mqttPublisherService.publishAck(deviceId, ackPayload);

      log.info(
          "Log enregistré et ACK publié : eventUuid={}, device={}",
          savedLog.eventUuid(), deviceId);

    } catch (Exception exception) {
      log.error(
          "Échec du traitement MQTT : topic={}, payload={}",
          topic, payload, exception);

      // Pas d'ACK positif si le traitement a échoué.
      // Le boîtier pourra appliquer sa stratégie de nouvelle tentative.
    }
  }

  private String extractDeviceId(String topic) {
    if (topic == null
        || !topic.startsWith(TOPIC_PREFIX)
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

    if (node == null || node.isNull() || node.asText().isBlank()) {
      throw new IllegalArgumentException(
          "Champ obligatoire absent ou vide : " + field);
    }

    return node.asText();
  }

  private Instant parseTimestamp(JsonNode node) {
    if (node == null || node.isNull()) {
      throw new IllegalArgumentException(
          "Champ timestamp absent.");
    }

    if (node.isNumber()) {
      // Hypothèse : une valeur numérique représente des millisecondes.
      return Instant.ofEpochMilli(node.asLong());
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
        // Si log_id n'est pas un UUID, créer un identifiant
        // déterministe pour le même événement et le même boîtier.
        return UUID.nameUUIDFromBytes(
            (deviceId + ":" + logId)
                .getBytes(StandardCharsets.UTF_8));
      }
    }

    // Sans identifiant fourni par le firmware, cette valeur est
    // déterministe pour ce payload. Le firmware devrait idéalement
    // fournir un identifiant stable pour gérer les retransmissions.
    return UUID.nameUUIDFromBytes(
        (deviceId + ":" + payload)
            .getBytes(StandardCharsets.UTF_8));
  }

  public record AckMessage(
      UUID eventUuid,
      String status,
      String message) {
  }
}
