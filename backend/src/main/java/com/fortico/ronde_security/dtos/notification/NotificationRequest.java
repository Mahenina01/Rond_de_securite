package com.fortico.ronde_security.dtos.notification;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Size;
public record NotificationRequest(

    @NotBlank (message = "Le titre est obligatoire")
    @Size(max = 200, message = "Le titre ne doit pas depasser 200 caractères") 
    String title,

    @NotBlank(message = "La description est obligatoire")
    String description,

    @NotBlank (message = "Le type est obligatoire")
    @Size(max=50, message =" le type ne doit pas depaser 50 caractères")
    String type
) {


}
