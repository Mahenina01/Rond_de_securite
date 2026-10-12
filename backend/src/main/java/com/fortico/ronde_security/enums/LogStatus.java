package com.fortico.ronde_security.enums;

public enum LogStatus {
  LOG_STATUS_PENDING, /// Écrit localement, envoi non confirmé.
  LOG_STATUS_SENT, /// Transmis et acquitté (ACK MQTT).
  LOG_STATUS_FAILED /// Échec du dernier envoi, à réessayer.

}
