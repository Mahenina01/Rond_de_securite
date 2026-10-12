
package com.fortico.ronde_security.controller;

import java.time.Instant;
import java.util.List;

import org.springframework.format.annotation.DateTimeFormat;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PatchMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import com.fortico.ronde_security.dtos.alerts.AlertsRequest;
import com.fortico.ronde_security.dtos.alerts.AlertsResponse;
import com.fortico.ronde_security.enums.AlertStatus;
import com.fortico.ronde_security.service.alerts.AlertsService;

import org.springframework.web.bind.annotation.RequestBody;
import jakarta.validation.Valid;

@RestController
@RequestMapping("/api/v1/alerts")
public class AlertsController {

  private final AlertsService alertsService;

  public AlertsController(AlertsService alertsService) {
    this.alertsService = alertsService;
  }

  @PostMapping
  public ResponseEntity<AlertsResponse> createAlert(
      @Valid @RequestBody AlertsRequest request) {

    AlertsResponse response = alertsService.createAlert(request);

    return ResponseEntity
        .status(HttpStatus.CREATED)
        .body(response);
  }

  @GetMapping
  public ResponseEntity<List<AlertsResponse>> getAlerts(
      @RequestParam(required = false) AlertStatus status,
      @RequestParam(name = "agent_id", required = false) Long agentId,
      @RequestParam(name = "location_id", required = false) Long locationId,
      @RequestParam(required = false) @DateTimeFormat(iso = DateTimeFormat.ISO.DATE_TIME) Instant from,
      @RequestParam(required = false) @DateTimeFormat(iso = DateTimeFormat.ISO.DATE_TIME) Instant to) {

    return ResponseEntity.ok(
        alertsService.getAlerts(status, agentId, locationId, from, to));
  }

  @GetMapping("/{id}")
  public ResponseEntity<AlertsResponse> getAlertById(
      @PathVariable Long id) {

    return ResponseEntity.ok(alertsService.getAlertById(id));
  }

  @PatchMapping("/{id}/acknowledge")
  public ResponseEntity<AlertsResponse> acknowledgeAlert(
      @PathVariable Long id) {

    return ResponseEntity.ok(
        alertsService.acknowledgeAlert(id));
  }

  @PatchMapping("/{id}/resolve")
  public ResponseEntity<AlertsResponse> resolveAlert(
      @PathVariable Long id) {

    return ResponseEntity.ok(
        alertsService.resolveAlert(id));
  }
}
