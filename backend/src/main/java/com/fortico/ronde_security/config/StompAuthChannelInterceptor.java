package com.fortico.ronde_security.config;

import org.springframework.messaging.Message;
import org.springframework.messaging.MessageChannel;
import org.springframework.messaging.simp.stomp.StompCommand;
import org.springframework.messaging.simp.stomp.StompHeaderAccessor;
import org.springframework.messaging.support.ChannelInterceptor;
import org.springframework.messaging.support.MessageHeaderAccessor;
import org.springframework.security.access.AccessDeniedException;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.core.userdetails.UsernameNotFoundException;
import org.springframework.stereotype.Component;

import com.fortico.ronde_security.filter.JwtUtils;

import io.jsonwebtoken.JwtException;

@Component
public class StompAuthChannelInterceptor implements ChannelInterceptor {

  private final JwtUtils jwtUtils;
  private final UserDetailsService userDetailsService;

  public StompAuthChannelInterceptor(
      JwtUtils jwtUtils,
      UserDetailsService userDetailsService) {
    this.jwtUtils = jwtUtils;
    this.userDetailsService = userDetailsService;
  }

  @Override
  public Message<?> preSend(Message<?> message, MessageChannel channel) {
    StompHeaderAccessor accessor =
        MessageHeaderAccessor.getAccessor(message, StompHeaderAccessor.class);

    if (accessor == null || accessor.getCommand() != StompCommand.CONNECT) {
      return message;
    }

    String authorization = accessor.getFirstNativeHeader("Authorization");
    if (authorization == null || !authorization.startsWith("Bearer ")) {
      throw new AccessDeniedException("Bearer token requis pour le WebSocket.");
    }

    String token = authorization.substring(7);
    String username;
    try {
      username = jwtUtils.extractUsername(token);
    } catch (JwtException | IllegalArgumentException exception) {
      throw new AccessDeniedException("Token WebSocket invalide.", exception);
    }

    UserDetails userDetails;
    try {
      userDetails = userDetailsService.loadUserByUsername(username);
    } catch (UsernameNotFoundException exception) {
      throw new AccessDeniedException("Utilisateur WebSocket introuvable.", exception);
    }

    if (!jwtUtils.isTokenValid(token, userDetails)
        || !userDetails.isEnabled()
        || !userDetails.isAccountNonLocked()
        || !userDetails.isAccountNonExpired()) {
      throw new AccessDeniedException("Authentification WebSocket refusée.");
    }

    accessor.setUser(new UsernamePasswordAuthenticationToken(
        userDetails,
        null,
        userDetails.getAuthorities()));
    return message;
  }
}
