package com.fortico.ronde_security.dtos.user;

import com.fortico.ronde_security.enums.Role;

public record UserResponse(
        Long id,
        String photo,
        String username,
        String name,
        String email,
        Role role,
        boolean active) {
}