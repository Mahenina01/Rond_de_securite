package com.fortico.ronde_security.dtos.alerts;

import java.time.Instant;

import com.fortico.ronde_security.enums.AlertStatus;

public record AlertsResponse(
    Long id,
    String message,
    Long locationId,
    AlertStatus status,
    Instant receivedAt) {
}