package com.fortico.ronde_security.service.logo;

import org.springframework.web.multipart.MultipartFile;

import com.fortico.ronde_security.dtos.logo.LogoResponse;

public interface LogoService {
    LogoResponse getLogo();

    LogoResponse createLogo(MultipartFile file);

    LogoResponse updateLogo(MultipartFile file);
}
