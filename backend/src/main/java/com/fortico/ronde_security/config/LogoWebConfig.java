package com.fortico.ronde_security.config;

import org.springframework.web.servlet.config.annotation.ResourceHandlerRegistry;
import org.springframework.web.servlet.config.annotation.WebMvcConfigurer;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Configuration;

import java.nio.file.Path;
import java.nio.file.Paths;

@Configuration
public class LogoWebConfig implements WebMvcConfigurer {
        private final String uploadDirectory;

        public LogoWebConfig(
                        @Value("${app.upload.logo-dir:uploads/logo}") String uploadDirectory) {
                this.uploadDirectory = uploadDirectory;
        }

        @Override
        public void addResourceHandlers(ResourceHandlerRegistry registry) {

                Path uploadPath = Paths.get(uploadDirectory)
                                .toAbsolutePath()
                                .normalize();

                String location = uploadPath.toUri().toString();

                registry.addResourceHandler("/uploads/logo/**")
                                .addResourceLocations(location);
        }
}
