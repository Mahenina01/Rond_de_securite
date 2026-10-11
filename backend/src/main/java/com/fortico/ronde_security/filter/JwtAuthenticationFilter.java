package com.fortico.ronde_security.filter;

import jakarta.servlet.FilterChain;
import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;

import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.web.authentication.WebAuthenticationDetailsSource;
import org.springframework.stereotype.Component;
import org.springframework.web.filter.OncePerRequestFilter;

import java.io.IOException;

@Component
public class JwtAuthenticationFilter extends OncePerRequestFilter {

  private final JwtUtils jwtUtils;
  private final UserDetailsService userDetailsService;

  public JwtAuthenticationFilter(
      JwtUtils jwtUtils,
      UserDetailsService userDetailsService) {
    this.jwtUtils = jwtUtils;
    this.userDetailsService = userDetailsService;
  }

  /**
   * Intercepte chaque requête HTTP une seule fois
   */
  @Override
  protected void doFilterInternal(
      HttpServletRequest request,
      HttpServletResponse response,
      FilterChain filterChain)
      throws ServletException, IOException {

    final String authorizationHeader = request.getHeader("Authorization");

    // Aucun token : laisser Spring Security appliquer ses règles
    if (authorizationHeader == null
        || !authorizationHeader.startsWith("Bearer ")) {
      filterChain.doFilter(request, response);
      return;
    }

    final String jwt = authorizationHeader.substring(7);

    try {
      final String username = jwtUtils.extractUsername(jwt);

      // Ne remplace pas une authentification déjà établie.
      if (username != null
          && SecurityContextHolder.getContext()
              .getAuthentication() == null) {

        UserDetails userDetails = userDetailsService.loadUserByUsername(username);

        if (jwtUtils.isTokenValid(jwt, userDetails)
            && userDetails.isEnabled()
            && userDetails.isAccountNonLocked()
            && userDetails.isAccountNonExpired()) {

          UsernamePasswordAuthenticationToken authentication = new UsernamePasswordAuthenticationToken(
              userDetails,
              null,
              userDetails.getAuthorities());

          authentication.setDetails(
              new WebAuthenticationDetailsSource()
                  .buildDetails(request));

          SecurityContextHolder.getContext()
              .setAuthentication(authentication);
        }
      }

    } catch (io.jsonwebtoken.JwtException
        | IllegalArgumentException ex) {
      // Token invalide, mal formé ou expiré :
      // ne pas authentifier la requête.
      SecurityContextHolder.clearContext();
    }

    filterChain.doFilter(request, response);
  }
}