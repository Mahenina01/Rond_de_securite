package com.fortico.ronde_security.dtos.logo;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Size;
public record Logorequest(
    @NotBlank(message = "Le chemin ou l'URL du logo est obligatoire.")
    @Size(max =500, message ="Le chemin ne doit pas dépasser 500 caractères")
    String photo
) {
    
}
