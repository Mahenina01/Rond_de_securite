package com.fortico.ronde_security.dtos.location;

public record LocationResponse(
    Long id,
    String checkpointCode,
    String name,
    Long agentId,
    Boolean isActive
){}
