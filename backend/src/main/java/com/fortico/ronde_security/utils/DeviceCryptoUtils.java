
package com.fortico.ronde_security.utils;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;
import java.util.HexFormat;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;

import org.springframework.stereotype.Component;

@Component
public class DeviceCryptoUtils {

  private static final int IV_LENGTH = 12;
  private static final int TAG_LENGTH_BITS = 128;

  private final SecureRandom secureRandom = new SecureRandom();
  private final SecretKey encryptionKey;

  public DeviceCryptoUtils(SecretKey deviceEncryptionKey) {
    this.encryptionKey = deviceEncryptionKey;
  }

  public String generateSecret() {
    byte[] secret = new byte[32];
    secureRandom.nextBytes(secret);

    return Base64.getUrlEncoder()
        .withoutPadding()
        .encodeToString(secret);
  }

  public String sha256Hex(String value) {
    try {
      byte[] hash = MessageDigest.getInstance("SHA-256")
          .digest(value.getBytes(StandardCharsets.UTF_8));

      return HexFormat.of().formatHex(hash);
    } catch (Exception e) {
      throw new IllegalStateException("Erreur SHA-256.", e);
    }
  }

  public boolean constantTimeEquals(String first, String second) {
    if (first == null || second == null) {
      return false;
    }

    return MessageDigest.isEqual(
        first.getBytes(StandardCharsets.UTF_8),
        second.getBytes(StandardCharsets.UTF_8));
  }

  public String encrypt(String plaintext) {
    byte[] iv = new byte[IV_LENGTH];
    secureRandom.nextBytes(iv);

    try {
      Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
      cipher.init(
          Cipher.ENCRYPT_MODE,
          encryptionKey,
          new GCMParameterSpec(TAG_LENGTH_BITS, iv));

      byte[] encrypted = cipher.doFinal(
          plaintext.getBytes(StandardCharsets.UTF_8));

      byte[] combined = new byte[iv.length + encrypted.length];

      System.arraycopy(iv, 0, combined, 0, iv.length);
      System.arraycopy(
          encrypted, 0, combined, iv.length, encrypted.length);

      return Base64.getEncoder().encodeToString(combined);

    } catch (Exception e) {
      throw new IllegalStateException(
          "Impossible de chiffrer le secret du dispositif.", e);
    }
  }

  public String decrypt(String encryptedValue) {
    byte[] combined;

    try {
      combined = Base64.getDecoder().decode(encryptedValue);
    } catch (IllegalArgumentException e) {
      throw new IllegalStateException(
          "Format du secret chiffré invalide.", e);
    }

    if (combined.length <= IV_LENGTH) {
      throw new IllegalStateException(
          "Données chiffrées incomplètes.");
    }

    byte[] iv = Arrays.copyOfRange(combined, 0, IV_LENGTH);
    byte[] encrypted = Arrays.copyOfRange(
        combined, IV_LENGTH, combined.length);

    try {
      Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
      cipher.init(
          Cipher.DECRYPT_MODE,
          encryptionKey,
          new GCMParameterSpec(TAG_LENGTH_BITS, iv));

      byte[] decrypted = cipher.doFinal(encrypted);

      return new String(decrypted, StandardCharsets.UTF_8);

    } catch (Exception e) {
      throw new IllegalStateException(
          "Impossible de déchiffrer le secret du dispositif.", e);
    }
  }
}
