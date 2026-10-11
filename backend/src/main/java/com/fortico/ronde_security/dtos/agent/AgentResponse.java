package com.fortico.ronde_security.dtos.agent;

public record AgentResponse(
        Long id,
        String agentCode,
        String name,
        String phone,
        Boolean isActive) {
}
