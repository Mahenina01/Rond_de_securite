package com.fortico.ronde_security.service.logo;

import java.util.Set;
import java.util.UUID;

import org.springframework.web.multipart.MultipartFile;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.nio.file.Files;
import java.io.IOException;

@Service
public class LogoStorageService {
    private static final Set<String> ALLOWED_TYPES = Set.of("image/jpeg", "image/png", "image/webp");

    private final Path uploadDirectory;

    public LogoStorageService(
            @Value("${app.upload.logo-dir:uploads/logo}") String uploadDirectory) throws IOException {
        this.uploadDirectory = Paths.get(uploadDirectory)
                .toAbsolutePath()
                .normalize();
        Files.createDirectories(this.uploadDirectory);
    }

    public String store(MultipartFile file) throws IOException {
        if (file == null || file.isEmpty()) {
            throw new IllegalArgumentException(
                    "Veuillez sélectioner une image.");
        }
        if (file.getSize() > 5L * 1024 * 1024) {
            throw new IllegalArgumentException(
                    "La taille du logo ne doit pas dépasser 5Mo");
        }

        String contentType = file.getContentType();
        if (contentType == null || !ALLOWED_TYPES.contains(contentType)) {
            throw new IllegalArgumentException(
                    "Format non autorisé. Utilisez JPEG, PNG ou WebP");
        }
        String originalName = file.getOriginalFilename();

        String extension = getExtension(originalName);

        if (extension == null) {
            throw new IllegalArgumentException(
                    "Extension de fichier non autorisée.");
        }
        String filename = UUID.randomUUID() + extension;
        Path destination = uploadDirectory.resolve(filename).normalize();
        if (!destination.startsWith((uploadDirectory))) {
            throw new IllegalArgumentException(
                    "Chemin de fichier invalide.");
        }
        Files.copy(file.getInputStream(), destination, StandardCopyOption.REPLACE_EXISTING);
        return "/uploads/logo" + filename;

    }

    private String getExtension(String filename) {
        if (filename == null) {
            return null;
        }
        String lower = filename.toLowerCase();
        if (lower.endsWith(".jpg") || lower.endsWith(".jpeg")) {
            return ".jpg";
        }
        if (lower.endsWith(".png")) {
            return ".png";
        }
        if (lower.endsWith(".webp")) {
            return ".webp";
        }
        return null;
    }

}
