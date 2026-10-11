package com.fortico.ronde_security.dtos.user;

import jakarta.validation.constraints.Email;
import jakarta.validation.constraints.Size;

public record UpdateProfileRequest(
        @Size(max = 500, message = "La photo ne doit pas dépasser 500 caractères") String photo,

        @Size(min = 3, max = 20, message = "Le nom d'utilisateur doit contenir entre 3 et 20 caractères") String username,
        @Size(min = 3, max = 100, message = "Le nom doit contenir entre 3 et 100 caractères") String name,

        @Email(message = "L'adresse e-mail est invalide") @Size(max = 255, message = "L'e-mail est trop long") String email) {
}
