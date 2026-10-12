package com.fortico.ronde_security.service.logo;

import java.io.IOException;
import org.springframework.http.HttpStatus;
import org.springframework.web.multipart.MultipartFile;
import org.springframework.web.server.ResponseStatusException;
import org.springframework.stereotype.Service;
import com.fortico.ronde_security.dtos.logo.LogoResponse;
import com.fortico.ronde_security.model.Logo;
import com.fortico.ronde_security.repo.LogoRepository;

import lombok.AllArgsConstructor;

import org.springframework.transaction.annotation.Transactional;
@Service
@AllArgsConstructor
public class LogoServiceImpl implements LogoService {

    private final LogoRepository logoRepository;
    private final LogoStorageService logoStorageService;

    @Override
    @Transactional(readOnly = true)
    public LogoResponse getLogo() {

        Logo logo = logoRepository.findAll()
                .stream()
                .findFirst()
                .orElseThrow(() -> new ResponseStatusException(
                        HttpStatus.NOT_FOUND,
                        "Aucun logo n'a encore été configuré."
                ));

        return toResponse(logo);
    }

    @Override
    public LogoResponse createLogo(MultipartFile file) {

        if (logoRepository.count() > 0) {
            throw new ResponseStatusException(
                    HttpStatus.CONFLICT,
                    "Un logo existe déjà. Utilisez PUT pour le remplacer."
            );
        }

        String photo = storeFile(file);

        Logo logo = Logo.builder()
                .photo(photo)
                .build();

        return toResponse(logoRepository.save(logo));
    }

    @Override
    public LogoResponse updateLogo(MultipartFile file) {

        Logo logo = logoRepository.findAll()
                .stream()
                .findFirst()
                .orElseThrow(() -> new ResponseStatusException(
                        HttpStatus.NOT_FOUND,
                        "Aucun logo n'existe. Créez d'abord le logo."
                ));

        String newPhoto = storeFile(file);

        logo.setPhoto(newPhoto);

        Logo saved = logoRepository.save(logo);

        // On ne supprime pas immédiatement l'ancien fichier :
        // il peut encore être utilisé par une requête en cours.
        // Un nettoyage contrôlé pourra être ajouté ensuite.

        return toResponse(saved);
    }

    private String storeFile(MultipartFile file) {

        try {
            return logoStorageService.store(file);
        } catch (IllegalArgumentException e) {
            throw new ResponseStatusException(
                    HttpStatus.BAD_REQUEST,
                    e.getMessage()
            );
        } catch (IOException e) {
            throw new ResponseStatusException(
                    HttpStatus.INTERNAL_SERVER_ERROR,
                    "Impossible d'enregistrer le fichier du logo."
            );
        }
    }

    private LogoResponse toResponse(Logo logo) {

        return new LogoResponse(
                logo.getId(),
                logo.getPhoto(),
                logo.getUpdatedAt()
        );
    }
}
