
package com.fortico.ronde_security.service.alerts;

import java.time.Instant;
import java.util.List;

import org.springframework.data.domain.Sort;
import org.springframework.data.jpa.domain.Specification;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import com.fortico.ronde_security.dtos.alerts.AlertsRequest;
import com.fortico.ronde_security.dtos.alerts.AlertsResponse;
import com.fortico.ronde_security.enums.AlertStatus;
import com.fortico.ronde_security.model.Alerts;
import com.fortico.ronde_security.model.Location;
import com.fortico.ronde_security.repo.AlertsRepository;
import com.fortico.ronde_security.repo.LocationRepository;
import com.fortico.ronde_security.repo.specification.AlertsSpecification;

import jakarta.persistence.EntityNotFoundException;
import lombok.AllArgsConstructor;

@Service
@Transactional(readOnly = true)
@AllArgsConstructor
public class AlertsServiceImpl implements AlertsService {

  private final AlertsRepository alertsRepository;

  private final AlertsRealtimeService alertsRealtimeService;
  private final LocationRepository locationRepository;

  @Override
  @Transactional
  public AlertsResponse createAlert(AlertsRequest request) {

    Location location = locationRepository.findById(request.locationId())
        .orElseThrow(() -> new EntityNotFoundException(
            "Emplacement introuvable : " + request.locationId()));

    Alerts alert = new Alerts();
    alert.setMessage(request.message());
    alert.setLocationId(location);
    alert.setStatus(request.status());

    Alerts savedAlert = alertsRepository.save(alert);

    AlertsResponse response = toResponse(savedAlert);

    // Diffusion de la nouvelle alerte en temps réel
    alertsRealtimeService.publishAlert(response);

    return response;
  }

  @Override
  public List<AlertsResponse> getAlerts(
      AlertStatus status,
      Long agentId,
      Long locationId,
      Instant from,
      Instant to) {

    if (from != null && to != null && from.isAfter(to)) {
      throw new IllegalArgumentException(
          "La date 'from' doit être antérieure ou égale à 'to'.");
    }

    Specification<Alerts> spec = Specification
        .where(AlertsSpecification.hasStatus(status))
        .and(AlertsSpecification.hasAgentId(agentId))
        .and(AlertsSpecification.hasLocationId(locationId))
        .and(AlertsSpecification.receivedAfter(from))
        .and(AlertsSpecification.receivedBefore(to));

    return alertsRepository
        .findAll(spec, Sort.by(Sort.Direction.DESC, "receivedAt"))
        .stream()
        .map(this::toResponse)
        .toList();
  }

  @Override
  public AlertsResponse getAlertById(Long id) {
    return toResponse(findAlert(id));
  }

  @Override
  @Transactional
  public AlertsResponse acknowledgeAlert(Long id) {
    Alerts alert = findAlert(id);
    alert.setStatus(AlertStatus.ACKNOWLEDGED);

    Alerts savedAlert = alertsRepository.save(alert);
    AlertsResponse response = toResponse(savedAlert);

    alertsRealtimeService.publishAlert(response);

    return response;
  }

  @Override
  @Transactional
  public AlertsResponse resolveAlert(Long id) {
    Alerts alert = findAlert(id);
    alert.setStatus(AlertStatus.RESOLVED);

    Alerts savedAlert = alertsRepository.save(alert);
    AlertsResponse response = toResponse(savedAlert);

    alertsRealtimeService.publishAlert(response);

    return response;
  }

  private Alerts findAlert(Long id) {
    return alertsRepository.findById(id)
        .orElseThrow(() -> new EntityNotFoundException(
            "Alerte introuvable avec l'identifiant : " + id));
  }

  private AlertsResponse toResponse(Alerts alert) {
    return new AlertsResponse(
        alert.getId(),
        alert.getMessage(),
        alert.getLocationId().getId(),
        alert.getStatus(),
        alert.getReceivedAt());
  }
}