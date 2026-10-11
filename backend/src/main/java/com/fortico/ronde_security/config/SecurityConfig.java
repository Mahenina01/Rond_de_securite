package com.fortico.ronde_security.config;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.http.HttpMethod;
import org.springframework.security.authentication.AuthenticationManager;
import org.springframework.security.config.Customizer;
import org.springframework.security.config.annotation.authentication.configuration.AuthenticationConfiguration;
import org.springframework.security.config.annotation.method.configuration.EnableMethodSecurity;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.configuration.EnableWebSecurity;
import org.springframework.security.config.http.SessionCreationPolicy;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.security.web.SecurityFilterChain;
import org.springframework.web.cors.CorsConfiguration;
import org.springframework.web.cors.CorsConfigurationSource;
import org.springframework.web.cors.UrlBasedCorsConfigurationSource;

import java.util.List;

@Configuration
@EnableWebSecurity
@EnableMethodSecurity
public class SecurityConfig {

  // Configuration principale de la sécurité HTTP

  @Bean
  public SecurityFilterChain securityFilterChain(
      HttpSecurity http) throws Exception {

    http
        // Autorise les requêtes provenant du frontend configuré ci-dessous
        .cors(Customizer.withDefaults())

        // API REST : désactivation du CSRF pour une API sans session navigateur
        .csrf(csrf -> csrf.disable())

        // Aucune session HTTP conservée côté serveur
        .sessionManagement(session -> session
            .sessionCreationPolicy(SessionCreationPolicy.STATELESS))

        // Règles d'accès aux endpoints.
        .authorizeHttpRequests(auth -> auth
            // Endpoints publics
            .requestMatchers(
                "/api/v1/users/login",
                "/actuator/health",
                "/error")
            .permitAll()

            // OPTIONS est utilisé notamment pour les requêtes CORS
            .requestMatchers(HttpMethod.OPTIONS, "/**").permitAll()

            // Tous les autres endpoints nécessitent une authentification.
            .anyRequest().authenticated())

        // Authentification HTTP Basic pour cette configuration initiale.
        .httpBasic(Customizer.withDefaults())

        // Désactive les formulaires de connexion HTML.
        .formLogin(form -> form.disable());

    return http.build();
  }

  /**
   * Encode les mots de passe avant leur enregistrement en base
   */
  @Bean
  public PasswordEncoder passwordEncoder() {
    return new BCryptPasswordEncoder();
  }

  /**
   * Fournit l'AuthenticationManager de Spring Security.
   */
  @Bean
  public AuthenticationManager authenticationManager(
      AuthenticationConfiguration configuration) throws Exception {
    return configuration.getAuthenticationManager();
  }

  // Configuration CORS
  @Bean
  public CorsConfigurationSource corsConfigurationSource() {
    CorsConfiguration configuration = new CorsConfiguration();

    configuration.setAllowedOrigins(List.of("http://localhost:5173")); // TODO : Remplace l'URL par celle de ton
                                                                       // application frontend

    configuration.setAllowedMethods(List.of(
        "GET", "POST", "PUT", "PATCH", "DELETE", "OPTIONS"));

    configuration.setAllowedHeaders(List.of(
        "Authorization", "Content-Type", "Accept"));

    configuration.setAllowCredentials(false);

    UrlBasedCorsConfigurationSource source = new UrlBasedCorsConfigurationSource();

    source.registerCorsConfiguration("/**", configuration);

    return source;
  }
}