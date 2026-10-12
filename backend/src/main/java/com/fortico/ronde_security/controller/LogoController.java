package com.fortico.ronde_security.controller;

import org.springframework.http.HttpStatus;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.multipart.MultipartFile;

import org.springframework.http.MediaType;
import org.springframework.validation.annotation.Validated;

import com.fortico.ronde_security.service.logo.LogoService;

import lombok.RequiredArgsConstructor;

import com.fortico.ronde_security.dtos.logo.*;
@RestController
@RequestMapping("/api/v1/logo")
@RequiredArgsConstructor 
@Validated 
public class LogoController {
    private final LogoService logoService;

    @GetMapping
    public LogoResponse getLogo() {
        return logoService.getLogo();
    }

    @PostMapping(consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
    @ResponseStatus(HttpStatus.CREATED)
    public LogoResponse createLogo(
            @RequestParam("file") MultipartFile file
    ) {
        return logoService.createLogo(file);
    }

    @PutMapping(consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
    public LogoResponse updateLogo(
            @RequestParam("file") MultipartFile file
    ) {
        return logoService.updateLogo(file);
    }
    
}
