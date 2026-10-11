package com.fortico.ronde_security.filter;

import io.jsonwebtoken.Claims;
import io.jsonwebtoken.JwtException;
import io.jsonwebtoken.Jwts;
import io.jsonwebtoken.io.Decoders;
import io.jsonwebtoken.security.Keys;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.stereotype.Component;

import javax.crypto.SecretKey;
import java.util.Date;
import java.util.function.Function;

@Component
public class JwtUtils {

  private final SecretKey signingKey;
  private final long jwtExpiration;

  public JwtUtils(
      @Value("${jwt.secret}") String secret,
      @Value("${jwt.expiration}") long jwtExpiration) {

    byte[] keyBytes = Decoders.BASE64.decode(secret);

    // La clé doit être suffisamment longue pour HMAC-SHA
    this.signingKey = Keys.hmacShaKeyFor(keyBytes);
    this.jwtExpiration = jwtExpiration;
  }

  /**
   * Génère un JWT à partir d'un utilisateur authentifié
   */
  public String generateToken(UserDetails userDetails) {
    Date now = new Date();
    Date expiration = new Date(
        now.getTime() + jwtExpiration);

    return Jwts.builder()
        .subject(userDetails.getUsername())
        .issuedAt(now)
        .expiration(expiration)
        .signWith(signingKey)
        .compact();
  }

  /**
   * Extrait le nom d'utilisateur du token
   */
  public String extractUsername(String token) {
    return extractClaim(token, Claims::getSubject);
  }

  /**
   * Extrait la date d'expiration du token
   */
  public Date extractExpiration(String token) {
    return extractClaim(token, Claims::getExpiration);
  }

  /**
   * Extrait une information précise du JWT
   */
  public <T> T extractClaim(
      String token,
      Function<Claims, T> claimsResolver) {

    Claims claims = extractAllClaims(token);
    return claimsResolver.apply(claims);
  }

  /**
   * Vérifie la signature et extrait les données du token.
   * Une exception est levée si le token est invalide ou expiré.
   */
  private Claims extractAllClaims(String token) {
    return Jwts.parser()
        .verifyWith(signingKey)
        .build()
        .parseSignedClaims(token)
        .getPayload();
  }

  /**
   * Vérifie que le token appartient à l'utilisateur
   * et qu'il n'est pas expiré.
   */
  public boolean isTokenValid(
      String token,
      UserDetails userDetails) {

    try {
      String username = extractUsername(token);

      return username.equals(userDetails.getUsername())
          && !isTokenExpired(token);

    } catch (JwtException | IllegalArgumentException ex) {
      return false;
    }
  }

  /**
   * Vérifie l'expiration du token.
   */
  private boolean isTokenExpired(String token) {
    Date expiration = extractExpiration(token);
    return expiration.before(new Date());
  }
}
