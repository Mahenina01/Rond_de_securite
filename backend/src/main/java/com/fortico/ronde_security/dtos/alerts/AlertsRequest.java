package com.fortico.ronde_security.dtos.alerts;

import com.fortico.ronde_security.enums.AlertStatus;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotNull;
import jakarta.validation.constraints.Positive;
import jakarta.validation.constraints.Size;

public record AlertsRequest(

    @NotBlank(message = "Le message est obligatoire") @Size(max = 5000) String message,

    @NotNull(message = "L'identifiant de l'emplacement est obligatoire") @Positive(message = "L'identifiant doit être positif") Long locationId,

    @NotNull(message = "Le statut est obligatoire") AlertStatus status

) {
}