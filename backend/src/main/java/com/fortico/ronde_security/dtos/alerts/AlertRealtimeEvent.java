package com.fortico.ronde_security.dtos.alerts;

public record AlertRealtimeEvent(
    Action action,
    Long alertId,
    AlertsResponse alert) {

  public enum Action {
    UPSERT,
    DELETE
  }
}
