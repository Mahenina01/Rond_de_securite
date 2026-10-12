package com.fortico.ronde_security.dtos.devices;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Size;

public record DevicesRequest(
    @NotBlank(message = "Le code de l'appareil est obligatoire") @Size(min = 3, max = 50, message = "Le code de l'appareil doit contenir entre 5 et 50 caractères") String deviceCode,
    @NotBlank(message = "Le nom de l'agent est obligatoire") @Size(min = 3, max = 100, message = "Le nom de l'agent doit contenir entre 2 et 100 caractères") String nameAgent) {

}
