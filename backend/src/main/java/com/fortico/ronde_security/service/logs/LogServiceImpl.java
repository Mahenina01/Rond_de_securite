
package com.fortico.ronde_security.service.logs;

import java.time.Instant;
import java.time.LocalDate;
import java.time.ZoneOffset;
import java.util.List;
import java.util.Set;

import org.springframework.data.domain.Sort;
import org.springframework.data.jpa.domain.Specification;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.web.server.ResponseStatusException;

import com.fortico.ronde_security.dtos.logs.LogsRequest;
import com.fortico.ronde_security.dtos.logs.LogsResponse;
import com.fortico.ronde_security.enums.LogStatus;
import com.fortico.ronde_security.model.Agents;
import com.fortico.ronde_security.model.Devices;
import com.fortico.ronde_security.model.Location;
import com.fortico.ronde_security.model.Logs;
import com.fortico.ronde_security.repo.AgentRepository;
import com.fortico.ronde_security.repo.DevicesRepository;
import com.fortico.ronde_security.repo.LocationRepository;
import com.fortico.ronde_security.repo.LogsRepository;

import jakarta.persistence.criteria.Predicate;
import lombok.AllArgsConstructor;

@Service
@Transactional
@AllArgsConstructor
public class LogServiceImpl implements LogService {

  private final LogsRepository logsRepository;
  private final AgentRepository agentRepository;
  private final DevicesRepository devicesRepository;
  private final LocationRepository locationRepository;

  // Créer un log
  @Override
  public LogsResponse createLog(LogsRequest request) {

    Agents agent = agentRepository
        .findByAgentCode(request.agentCode())
        .orElseThrow(() -> new ResponseStatusException(
            HttpStatus.NOT_FOUND,
            "Agent introuvable : " + request.agentCode()));

    Devices device = devicesRepository
        .findByDeviceCode(request.deviceCode())
        .orElseThrow(() -> new ResponseStatusException(
            HttpStatus.NOT_FOUND,
            "Appareil introuvable : " + request.deviceCode()));

    Location location = locationRepository
        .findById(request.locationId())
        .orElseThrow(() -> new ResponseStatusException(
            HttpStatus.NOT_FOUND,
            "Emplacement introuvable : " + request.locationId()));

    Logs log = Logs.builder()
        .eventUuid(request.eventUuid())
        .agent(agent)
        .device(device)
        .locationId(location)
        .status(request.status())
        .timestamp(request.timestamp())
        .build();

    Logs savedLog = logsRepository.save(log);

    return toResponse(savedLog);
  }

  // Lister et filtrer les logs
  @Override
  @Transactional(readOnly = true)
  public List<LogsResponse> getLogs(
      LogStatus status,
      LocalDate date,
      Long agentId,
      Long deviceId,
      Long locationId,
      Instant from,
      Instant to,
      String sort,
      String order) {

    if (from != null && to != null && from.isAfter(to)) {
      throw new ResponseStatusException(
          HttpStatus.BAD_REQUEST,
          "La date 'from' doit être antérieure ou égale à 'to'.");
    }

    if (date != null && (from != null || to != null)) {
      throw new ResponseStatusException(
          HttpStatus.BAD_REQUEST,
          "Utilisez soit 'date', soit 'from' et 'to'.");
    }

    Specification<Logs> spec = (root, query, cb) -> {
      var predicates = new java.util.ArrayList<Predicate>();

      if (status != null) {
        predicates.add(cb.equal(root.get("status"), status));
      }

      if (date != null) {
        Instant start = date.atStartOfDay()
            .toInstant(ZoneOffset.UTC);

        Instant end = date.plusDays(1)
            .atStartOfDay()
            .toInstant(ZoneOffset.UTC);

        predicates.add(cb.greaterThanOrEqualTo(
            root.get("timestamp"), start));

        predicates.add(cb.lessThan(
            root.get("timestamp"), end));
      }

      if (agentId != null) {
        predicates.add(cb.equal(
            root.get("agent").get("id"), agentId));
      }

      if (deviceId != null) {
        predicates.add(cb.equal(
            root.get("device").get("id"), deviceId));
      }

      if (locationId != null) {
        predicates.add(cb.equal(
            root.get("locationId").get("id"), locationId));
      }

      if (from != null) {
        predicates.add(cb.greaterThanOrEqualTo(
            root.get("timestamp"), from));
      }

      if (to != null) {
        predicates.add(cb.lessThanOrEqualTo(
            root.get("timestamp"), to));
      }

      return cb.and(predicates.toArray(new Predicate[0]));
    };

    // Autoriser uniquement les champs de tri prévus
    Set<String> allowedSortFields = Set.of(
        "id", "timestamp", "receivedAt", "status",
        "agentCode", "deviceCode", "locationId");

    String sortField = (sort == null || sort.isBlank())
        ? "timestamp"
        : sort;

    if (!allowedSortFields.contains(sortField)) {
      throw new ResponseStatusException(
          HttpStatus.BAD_REQUEST,
          "Champ de tri invalide : " + sortField);
    }

    Sort.Direction direction = "asc".equalsIgnoreCase(order)
        ? Sort.Direction.ASC
        : "desc".equalsIgnoreCase(order) || order == null
            || order.isBlank()
                ? Sort.Direction.DESC
                : null;

    if (direction == null) {
      throw new ResponseStatusException(
          HttpStatus.BAD_REQUEST,
          "L'ordre doit être 'asc' ou 'desc'.");
    }

    String sortProperty = switch (sortField) {
      case "agentCode" -> "agent.agentCode";
      case "deviceCode" -> "device.deviceCode";
      case "locationId" -> "locationId.id";
      default -> sortField;
    };

    Sort sorting = Sort.by(direction, sortProperty);

    return logsRepository.findAll(spec, sorting)
        .stream()
        .map(this::toResponse)
        .toList();
  }

  // Récupérer un log par son ID
  @Override
  @Transactional(readOnly = true)
  public LogsResponse getLogById(Long id) {

    Logs log = logsRepository.findById(id)
        .orElseThrow(() -> new ResponseStatusException(
            HttpStatus.NOT_FOUND,
            "Log introuvable avec l'identifiant : " + id));

    return toResponse(log);
  }

  // Convertir l'entité en DTO de réponse
  private LogsResponse toResponse(Logs log) {

    return new LogsResponse(
        log.getId(),
        log.getEventUuid(),
        log.getAgent().getAgentCode(),
        log.getDevice().getDeviceCode(),
        log.getLocationId().getId(),
        log.getStatus(),
        log.getTimestamp(),
        log.getReceivedAt());
  }
}
