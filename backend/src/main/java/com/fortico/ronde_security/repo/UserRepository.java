package com.fortico.ronde_security.repo;

import org.springframework.data.jpa.repository.JpaRepository;

import com.fortico.ronde_security.model.Users;

public interface UserRepository extends JpaRepository<Users, Long> {
  Users findByUsername(String username);

  Users findByEmail(String email);
}
