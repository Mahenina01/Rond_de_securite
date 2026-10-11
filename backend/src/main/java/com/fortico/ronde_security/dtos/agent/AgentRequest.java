package com.fortico.ronde_security.dtos.agent;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Size;

public record AgentRequest(
                @NotBlank(message = "Le code de l'angent est obligatoire") @Size(max = 50) String agentCode,

                @NotBlank(message = "Le nom de l'angent est obligatoire") @Size(max = 150) String name,

                @Size(max = 15) String phone) {
}
