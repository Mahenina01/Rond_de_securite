
package com.fortico.ronde_security.dtos.logs;

import java.time.Instant;
import java.util.UUID;

import com.fortico.ronde_security.enums.LogStatus;
import jakarta.validation.constraints.NotNull;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.PastOrPresent;
import jakarta.validation.constraints.Positive;

public record LogsRequest(

    UUID eventUuid,

    @NotBlank(message = "Le code de l'agent est obligatoire") String agentCode,

    @NotBlank(message = "Le code de l'appareil est obligatoire") String deviceCode,

    @NotNull(message = "L'identifiant de l'emplacement est obligatoire") @Positive(message = "L'identifiant de l'emplacement doit être positif") Long locationId,

    @NotNull(message = "Le statut du log est obligatoire") LogStatus status,

    @NotNull(message = "La date du scan est obligatoire") @PastOrPresent(message = "La date du scan ne peut pas être dans le futur") Instant timestamp

) {
}
