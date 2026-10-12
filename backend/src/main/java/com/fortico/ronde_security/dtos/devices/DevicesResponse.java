package com.fortico.ronde_security.dtos.devices;

public record DevicesResponse(
        Long id,
        String deviceCode,
        String agentCode,
        Boolean isActive) {
}