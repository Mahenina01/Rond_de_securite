package com.fortico.ronde_security.model;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.Id;
import lombok.Data;

@Entity(name = "users")
@Data
public class Users {

  @Id
  @GeneratedValue(strategy = jakarta.persistence.GenerationType.IDENTITY)
  private Long id;

  @Column(name = "photo", length = 500)
  private String photo;

  @Column(name = "username", nullable = false, unique = true, length = 100)
  private String username;

  @Column(name = "email", nullable = false, unique = true, length = 255)
  private String email;

  @Column(name = "password", nullable = false, length = 255)
  private String password;

  @Column(name = "role", nullable = false, length = 30)
  private String role;

  @Column(name = "is_active", nullable = false)
  private boolean isActive;
}
