package com.fortico.ronde_security.dtos;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Size;

public record AgentRequest (
    @NotBlank(message = "Le code de l'angent est obligatoire")
    @Size(max = 50)
    String agent_code,

    @NotBlank(message = "Le nom de l'angent est obligatoire")
    @Size(max = 150)
    String name,

    @Size(max = 15)
    String phone
){}
