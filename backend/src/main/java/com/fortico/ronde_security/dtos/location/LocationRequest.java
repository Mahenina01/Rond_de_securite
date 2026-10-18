package com.fortico.ronde_security.dtos.location;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Size;

public record LocationRequest(

    @NotBlank(message = "Le code du point de contrôle est obligatoire") @Size(max = 50) String checkpointCode,

    @NotBlank(message = "Le nom de l'emplacement est obligatoire") @Size(max = 100) String name,

    @Size(max = 100) String agentName

) {
}