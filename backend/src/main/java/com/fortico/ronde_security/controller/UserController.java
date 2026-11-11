
package com.fortico.ronde_security.controller;

import com.fortico.ronde_security.dtos.user.*;
import com.fortico.ronde_security.service.UserService;

import jakarta.validation.Valid;

import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.MediaTypeFactory;
import org.springframework.http.ResponseEntity;
import org.springframework.core.io.Resource;
import org.springframework.security.core.annotation.AuthenticationPrincipal;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.web.multipart.MultipartFile;
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

  @PutMapping(value = "/me/photo", consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
  public ResponseEntity<UserResponse> updateProfilePhoto(
      @AuthenticationPrincipal UserDetails principal,
      @RequestParam("file") MultipartFile file) {

    return ResponseEntity.ok(
        userService.updateProfilePhoto(principal.getUsername(), file));
  }

  @GetMapping("/me/photo")
  public ResponseEntity<Resource> getProfilePhoto(
      @AuthenticationPrincipal UserDetails principal) {

    Resource photo = userService.getProfilePhoto(principal.getUsername());
    String filename = photo.getFilename();
    MediaType mediaType = filename != null && filename.toLowerCase().endsWith(".webp")
        ? MediaType.parseMediaType("image/webp")
        : MediaTypeFactory.getMediaType(filename == null ? "" : filename)
            .orElse(MediaType.APPLICATION_OCTET_STREAM);

    return ResponseEntity.ok()
        .contentType(mediaType)
        .body(photo);
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
