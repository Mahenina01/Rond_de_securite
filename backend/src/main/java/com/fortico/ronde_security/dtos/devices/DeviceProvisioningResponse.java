
package com.fortico.ronde_security.dtos.devices;

public record DeviceProvisioningResponse(
    Long id,
    String deviceCode,
    String agentCode,
    Boolean isActive,
    String secret) {
}
