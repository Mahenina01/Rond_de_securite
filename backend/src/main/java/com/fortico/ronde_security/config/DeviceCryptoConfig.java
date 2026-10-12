package com.fortico.ronde_security.config;

import java.util.Base64;

import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

@Configuration
public class DeviceCryptoConfig {

  @Bean
  public SecretKey deviceEncryptionKey(
      @Value("${security.device.encryption-key}") String base64Key) {

    byte[] decoded = Base64.getDecoder().decode(base64Key);

    if (decoded.length != 32) {
      throw new IllegalArgumentException(
          "La clé de chiffrement doit contenir exactement 32 octets.");
    }

    return new SecretKeySpec(decoded, "AES");
  }
}
