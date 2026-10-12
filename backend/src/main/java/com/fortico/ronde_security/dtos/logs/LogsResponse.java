
package com.fortico.ronde_security.dtos.logs;

import java.time.Instant;
import java.util.UUID;

import com.fortico.ronde_security.enums.LogStatus;

public record LogsResponse(

    Long id,
    UUID eventUuid,
    String agentCode,
    String deviceCode,
    Long locationId,
    LogStatus status,
    Instant timestamp,
    Instant receivedAt

) {
}
