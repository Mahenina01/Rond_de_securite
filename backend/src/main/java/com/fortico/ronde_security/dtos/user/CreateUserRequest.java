package com.fortico.ronde_security.dtos.user;

import jakarta.validation.constraints.Email;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Size;
import lombok.Data;

@Data
public class CreateUserRequest {

  @NotBlank(message = "Le nom d'utilisateur est obligatoire")
  @Size(min = 3, max = 20)
  private String username;

  @NotBlank(message = "Le nom est obligatoire")
  @Size(min = 3, max = 100)
  private String name;

  @NotBlank(message = "L'adresse email est obligatoire")
  @Email(message = "L'adresse email est invalide")
  @Size(max = 255)
  private String email;

  @Size(max = 500)
  private String photo;

  @NotBlank(message = "Le mot de passe est obligatoire")
  @Size(min = 8, max = 72, message = "Le mot de passe doit être supperieur à 8 caracteres")
  private String password;
}
