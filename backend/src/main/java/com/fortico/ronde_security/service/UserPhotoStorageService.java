package com.fortico.ronde_security.service;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.Locale;
import java.util.Set;
import java.util.UUID;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.core.io.Resource;
import org.springframework.core.io.UrlResource;
import org.springframework.stereotype.Service;
import org.springframework.web.multipart.MultipartFile;

@Service
public class UserPhotoStorageService {
  private static final String DEFAULT_PHOTO = "user.webp";
  private static final long MAX_FILE_SIZE = 5L * 1024 * 1024;
  private static final Set<String> ALLOWED_EXTENSIONS = Set.of(".jpg", ".jpeg", ".png", ".webp");

  private final Path uploadDirectory;

  public UserPhotoStorageService(
      @Value("${app.upload.user-dir:uploads/user}") String uploadDirectory) throws IOException {
    this.uploadDirectory = Paths.get(uploadDirectory).toAbsolutePath().normalize();
    Files.createDirectories(this.uploadDirectory);
  }

  public String store(MultipartFile file) throws IOException {
    if (file == null || file.isEmpty()) {
      throw new IllegalArgumentException("Veuillez sélectionner une image.");
    }
    if (file.getSize() > MAX_FILE_SIZE) {
      throw new IllegalArgumentException("La photo ne doit pas dépasser 5 Mo.");
    }

    String originalName = file.getOriginalFilename();
    int extensionSeparator = originalName == null ? -1 : originalName.lastIndexOf('.');
    String extension = extensionSeparator < 0
        ? ""
        : originalName.substring(extensionSeparator).toLowerCase(Locale.ROOT);
    String contentType = file.getContentType();

    if (!ALLOWED_EXTENSIONS.contains(extension)
        || !isAllowedContentType(extension, contentType)) {
      throw new IllegalArgumentException("Format non autorisé. Utilisez JPEG, PNG ou WebP.");
    }

    String filename = UUID.randomUUID() + extension;
    Path destination = uploadDirectory.resolve(filename).normalize();
    if (!destination.startsWith(uploadDirectory)) {
      throw new IllegalArgumentException("Chemin de fichier invalide.");
    }

    try (var inputStream = file.getInputStream()) {
      Files.copy(inputStream, destination, StandardCopyOption.REPLACE_EXISTING);
    }
    return filename;
  }

  public Resource load(String filename) throws IOException {
    String safeFilename = filename == null || filename.isBlank()
        ? DEFAULT_PHOTO
        : Paths.get(filename).getFileName().toString();
    Path photoPath = uploadDirectory.resolve(safeFilename).normalize();
    if (!photoPath.startsWith(uploadDirectory) || !Files.isRegularFile(photoPath)) {
      throw new IOException("La photo de profil est introuvable.");
    }

    return new UrlResource(photoPath.toUri());
  }

  private boolean isAllowedContentType(String extension, String contentType) {
    if (contentType == null) {
      return false;
    }
    return switch (extension) {
      case ".jpg", ".jpeg" -> contentType.equals("image/jpeg");
      case ".png" -> contentType.equals("image/png");
      case ".webp" -> contentType.equals("image/webp");
      default -> false;
    };
  }
}
