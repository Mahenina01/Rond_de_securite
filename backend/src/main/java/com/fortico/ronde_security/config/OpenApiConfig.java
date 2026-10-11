package com.fortico.ronde_security.config;

import io.swagger.v3.oas.annotations.OpenAPIDefinition;
import io.swagger.v3.oas.annotations.info.Contact;
import io.swagger.v3.oas.annotations.info.Info;
import org.springframework.context.annotation.Configuration;

@Configuration
@OpenAPIDefinition(info = @Info(title = "Ronde Security API", version = "1.0.0", description = """
    API REST de gestion des rondes de sécurité,
    des dispositifs, des points de contrôle RFID
    et des données de passage.
    """, contact = @Contact(name = "Équipe Ronde Security")))
public class OpenApiConfig {
}
