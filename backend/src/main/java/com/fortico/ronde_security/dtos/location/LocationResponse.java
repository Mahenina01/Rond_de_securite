package com.fortico.ronde_security.dtos.location;

import com.fortico.ronde_security.dtos.agent.AgentResponse;

public record LocationResponse(
        Long id,
        String checkpointCode,
        String name,
        AgentResponse agent,
        Boolean isActive) {
}
