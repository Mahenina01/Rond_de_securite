package com.fortico.ronde_security.dtos.location;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Positive;
import jakarta.validation.constraints.Size;

public record LocationRequest(
        @NotBlank(message = "Le code du point de contrôle est obligatoire") @Size(max = 50) String checkpointCode,

        @NotBlank(message = "Le code du point de contrôle est obligatoire") @Size(max = 50) String name,

        @Positive(message = "L'identifiant de l'agent doit être un nombre positif") Long agentId) {
}