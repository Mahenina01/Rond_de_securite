
package com.fortico.ronde_security.controler;

import com.fortico.ronde_security.dtos.user.*;
import com.fortico.ronde_security.service.UserService;

import jakarta.validation.Valid;

import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.annotation.AuthenticationPrincipal;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/api/v1/users")
public class UserController {

  private final UserService userService;

  public UserController(UserService userService) {
    this.userService = userService;
  }

  // POST /api/v1/users/create
  @PostMapping("/create")
  public ResponseEntity<UserResponse> createUser(
      @Valid @RequestBody CreateUserRequest request) {

    UserResponse response = userService.createUser(request);

    return ResponseEntity.status(HttpStatus.CREATED).body(response);
  }

  // POST /api/v1/users/login
  @PostMapping("/login")
  public ResponseEntity<LoginResponse> login(
      @Valid @RequestBody LoginRequest request) {

    return ResponseEntity.ok(userService.login(request));
  }

  // POST /api/v1/users/logout
  @PostMapping("/logout")
  public ResponseEntity<Void> logout() {
    // JWT sans état : le client doit supprimer son token.
    return ResponseEntity.noContent().build();
  }

  // GET /api/v1/users/me
  @GetMapping("/me")
  public ResponseEntity<UserResponse> getMe(
      @AuthenticationPrincipal UserDetails principal) {

    return ResponseEntity.ok(
        userService.getMe(principal.getUsername()));
  }

  // PATCH /api/v1/users/me
  @PatchMapping("/me")
  public ResponseEntity<UserResponse> updateProfile(
      @AuthenticationPrincipal UserDetails principal,
      @Valid @RequestBody UpdateProfileRequest request) {

    return ResponseEntity.ok(
        userService.updateProfile(
            principal.getUsername(),
            request));
  }

  // PATCH /api/v1/users/me/password
  @PatchMapping("/me/password")
  public ResponseEntity<Void> changePassword(
      @AuthenticationPrincipal UserDetails principal,
      @Valid @RequestBody ChangePasswordRequest request) {

    userService.changePassword(
        principal.getUsername(),
        request);

    return ResponseEntity.noContent().build();
  }

  // DELETE /api/v1/users/{id}
  @DeleteMapping("/delete/{id}")
  public ResponseEntity<Void> deleteUser(@PathVariable Long id) {
    userService.deleteUser(id);
    return ResponseEntity.noContent().build();
  }
}
