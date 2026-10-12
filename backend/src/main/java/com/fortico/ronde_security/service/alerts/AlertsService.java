
package com.fortico.ronde_security.service.alerts;

import java.time.Instant;
import java.util.List;

import com.fortico.ronde_security.dtos.alerts.AlertsRequest;
import com.fortico.ronde_security.dtos.alerts.AlertsResponse;
import com.fortico.ronde_security.enums.AlertStatus;

public interface AlertsService {

  List<AlertsResponse> getAlerts(
      AlertStatus status,
      Long agentId,
      Long locationId,
      Instant from,
      Instant to);

  AlertsResponse getAlertById(Long id);

  AlertsResponse acknowledgeAlert(Long id);

  AlertsResponse resolveAlert(Long id);

  AlertsResponse createAlert(AlertsRequest request);
}