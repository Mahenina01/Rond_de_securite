
package com.fortico.ronde_security.dtos.alerts;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotNull;
import jakarta.validation.constraints.Positive;
import jakarta.validation.constraints.Size;

import com.fortico.ronde_security.enums.AlertStatus;

public record AlertsRequest(

    @NotBlank(message = "Le message est obligatoire") @Size(max = 5000, message = "Le message ne doit pas dépasser 5000 caractères") String message,

    @NotNull(message = "L'identifiant de l'emplacement est obligatoire") @Positive(message = "L'identifiant de l'emplacement doit être positif") Long locationId,

    @NotNull(message = "Le statut est obligatoire") AlertStatus status

) {
}