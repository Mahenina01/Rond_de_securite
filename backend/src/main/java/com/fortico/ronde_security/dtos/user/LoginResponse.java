package com.fortico.ronde_security.dtos.user;

public record LoginResponse(
    String token,
    String tokenType,
    UserResponse user) {
}