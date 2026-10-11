package com.fortico.ronde_security.repo;

import java.util.Optional;

import org.springframework.data.jpa.repository.JpaRepository;

import com.fortico.ronde_security.model.Users;

public interface UserRepository extends JpaRepository<Users, Long> {
  Optional<Users> findByUsername(String username);

  Optional<Users> findById(Long id);

  boolean existsByUsername(String username);

  boolean existsByEmail(String email);
}
