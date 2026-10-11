package com.fortico.ronde_security.tdos.user;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Size;
import lombok.Data;

@Data
public class LoginRequest {

  @NotBlank(message = "Email is required")
  @Size(max = 255, message = "Email must not exceed 255 characters")
  private String email;

  @NotBlank(message = "Password is required")
  @Size(max = 255, message = "Password must not exceed 255 characters")
  private String password;

}
