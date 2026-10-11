package com.fortico.ronde_security.dtos;

import java.time.OffsetDateTime;

public record AgentResponse(
        Long id,
        String agent_code,
        String name,
        String phone,
        Boolean is_active,
        OffsetDateTime created_at
) {}
