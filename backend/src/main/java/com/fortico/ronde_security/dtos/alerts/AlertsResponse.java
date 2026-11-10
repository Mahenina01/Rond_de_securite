package com.fortico.ronde_security.dtos.alerts;

import java.time.Instant;

import com.fortico.ronde_security.enums.AlertStatus;

public record AlertsResponse(
    Long id,
    String message,
    Long locationId,
    AlertStatus status,
    Instant receivedAt,
    Instant timestamp,
    LocationDetails location,
    AgentDetails agent) {

  public record LocationDetails(String name) {
  }

  public record AgentDetails(
      Long id,
      String agent_code,
      String name,
      String phone) {
  }
}