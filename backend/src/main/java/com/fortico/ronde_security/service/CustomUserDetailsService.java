package com.fortico.ronde_security.service;

import com.fortico.ronde_security.model.Users;
import com.fortico.ronde_security.repo.UserRepository;

import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.core.userdetails.UsernameNotFoundException;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

@Service
public class CustomUserDetailsService implements UserDetailsService {

  private final UserRepository userRepository;

  public CustomUserDetailsService(UserRepository userRepository) {
    this.userRepository = userRepository;
  }

  @Override
  @Transactional(readOnly = true)
  public UserDetails loadUserByUsername(String username)
      throws UsernameNotFoundException {

    Users user = userRepository.findByUsername(username)
        .orElseThrow(() -> new UsernameNotFoundException(
            "Utilisateur introuvable : " + username));

    return org.springframework.security.core.userdetails.User
        .withUsername(user.getUsername())
        .password(user.getPassword())
        .authorities("ROLE_" + user.getRole().name())
        .disabled(!user.isActive())
        .build();
  }
}