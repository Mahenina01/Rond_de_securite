
package com.fortico.ronde_security.service;

import org.springframework.security.authentication.AuthenticationManager;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import com.fortico.ronde_security.dtos.user.*;
import com.fortico.ronde_security.enums.Role;
import com.fortico.ronde_security.exception.DuplicateResourceException;
import com.fortico.ronde_security.exception.ResourceNotFoundException;
import com.fortico.ronde_security.model.Users;
import com.fortico.ronde_security.repo.UserRepository;
import com.fortico.ronde_security.filter.JwtUtils;

@Service
public class UserService {

  private final UserRepository userRepository;
  private final AuthenticationManager authenticationManager;
  private final PasswordEncoder passwordEncoder;
  private final JwtUtils jwtUtils;

  public UserService(
      UserRepository userRepository,
      AuthenticationManager authenticationManager,
      PasswordEncoder passwordEncoder,
      JwtUtils jwtUtils) {
    this.userRepository = userRepository;
    this.authenticationManager = authenticationManager;
    this.passwordEncoder = passwordEncoder;
    this.jwtUtils = jwtUtils;
  }

  @Transactional
  public UserResponse createUser(CreateUserRequest request) {

    if (userRepository.existsByUsername(request.getUsername())) {
      throw new DuplicateResourceException(
          "Ce nom d'utilisateur existe déjà.");
    }

    if (userRepository.existsByEmail(request.getEmail())) {
      throw new DuplicateResourceException(
          "Cette adresse email existe déjà.");
    }

    Users user = new Users();
    user.setUsername(request.getUsername().trim());
    user.setEmail(request.getEmail().trim());
    user.setPhoto(request.getPhoto());
    user.setName(request.getName().trim());

    // Encoder le mot de passe avant l'enregistrement
    user.setPassword(
        passwordEncoder.encode(request.getPassword()));

    // Les valeurs sensibles sont définies côté serveur
    user.setRole(Role.ADMIN);
    user.setActive(true);

    Users savedUser = userRepository.save(user);

    return toUserResponse(savedUser);
  }

  // Supprimer un utilisateur
  @Transactional
  public void deleteUser(Long id) {
    Users user = userRepository.findById(id).orElseThrow(() -> new ResourceNotFoundException("Utilisateur non trouvé"));
    userRepository.delete(user);
  }

  public LoginResponse login(LoginRequest request) {
    Authentication authentication = authenticationManager.authenticate(
        new UsernamePasswordAuthenticationToken(
            request.username(),
            request.password()));

    UserDetails userDetails = (UserDetails) authentication.getPrincipal();

    Users user = userRepository.findByUsername(
        userDetails.getUsername())
        .orElseThrow(() -> new org.springframework.security.core.userdetails.UsernameNotFoundException(
            "Utilisateur introuvable"));

    String token = jwtUtils.generateToken(userDetails);

    return new LoginResponse(
        token,
        "Bearer",
        toUserResponse(user));
  }

  @Transactional(readOnly = true)
  public UserResponse getMe(String username) {
    return toUserResponse(findUser(username));
  }

  @Transactional
  public UserResponse updateProfile(
      String authenticatedUsername,
      UpdateProfileRequest request) {

    Users user = findUser(authenticatedUsername);

    if (request.username() != null) {
      String username = request.username().trim();

      if (username.isBlank()) {
        throw new IllegalArgumentException(
            "Le nom d'utilisateur ne peut pas être vide");
      }

      if (!username.equals(user.getUsername())
          && userRepository.existsByUsername(username)) {
        throw new IllegalArgumentException(
            "Ce nom d'utilisateur est déjà utilisé");
      }

      user.setUsername(username);
    }

    if (request.email() != null) {
      String email = request.email().trim();

      if (email.isBlank()) {
        throw new IllegalArgumentException(
            "L'adresse e-mail ne peut pas être vide");
      }

      if (!email.equalsIgnoreCase(user.getEmail())
          && userRepository.existsByEmail(email)) {
        throw new IllegalArgumentException(
            "Cette adresse e-mail est déjà utilisée");
      }

      user.setEmail(email);
    }

    if (request.photo() != null) {
      user.setPhoto(request.photo().trim());
    }

    return toUserResponse(userRepository.save(user));
  }

  @Transactional
  public void changePassword(
      String username,
      ChangePasswordRequest request) {

    Users user = findUser(username);

    if (!passwordEncoder.matches(
        request.currentPassword(), user.getPassword())) {
      throw new IllegalArgumentException(
          "Le mot de passe actuel est incorrect");
    }

    if (passwordEncoder.matches(
        request.newPassword(), user.getPassword())) {
      throw new IllegalArgumentException(
          "Le nouveau mot de passe doit être différent de l'ancien");
    }

    user.setPassword(
        passwordEncoder.encode(request.newPassword()));

    userRepository.save(user);
  }

  private Users findUser(String username) {
    return userRepository.findByUsername(username)
        .orElseThrow(() -> new org.springframework.security.core.userdetails.UsernameNotFoundException(
            "Utilisateur introuvable"));
  }

  private UserResponse toUserResponse(Users user) {
    return new UserResponse(
        user.getId(),
        user.getPhoto(),
        user.getUsername(),
        user.getName(),
        user.getEmail(),
        user.getRole(),
        user.isActive());
  }
}
