
package com.fortico.ronde_security.utils;

import java.nio.charset.StandardCharsets;
import java.util.Base64;

import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;

import org.springframework.stereotype.Component;

@Component
public class HmacUtils {

  public String sign(String secret, String canonicalMessage) {
    try {
      byte[] secretBytes = Base64.getUrlDecoder().decode(secret);

      Mac mac = Mac.getInstance("HmacSHA256");
      mac.init(new SecretKeySpec(secretBytes, "HmacSHA256"));

      byte[] signature = mac.doFinal(
          canonicalMessage.getBytes(StandardCharsets.UTF_8));

      return Base64.getEncoder().encodeToString(signature);

    } catch (Exception e) {
      throw new IllegalStateException(
          "Erreur lors du calcul de la signature HMAC.", e);
    }
  }

  public boolean verify(
      String secret,
      String canonicalMessage,
      String suppliedSignature) {

    if (secret == null || canonicalMessage == null
        || suppliedSignature == null) {
      return false;
    }

    try {
      byte[] expected = Base64.getDecoder().decode(
          sign(secret, canonicalMessage));

      byte[] supplied = Base64.getDecoder().decode(
          suppliedSignature);

      return java.security.MessageDigest.isEqual(expected, supplied);

    } catch (IllegalArgumentException e) {
      return false;
    }
  }
}
