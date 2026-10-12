package com.fortico.ronde_security.controller;

import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.*;

import lombok.AllArgsConstructor;
import lombok.RequiredArgsConstructor;
import com.fortico.ronde_security.service.notifications.NotificationService;
import com.fortico.ronde_security.dtos.notification.NotificationRequest;
import com.fortico.ronde_security.dtos.notification.NotificationResponse;
import jakarta.validation.Valid;
import jakarta.validation.constraints.Positive;

import org.springframework.http.HttpStatus;
import java.util.List;
@RestController 
@RequestMapping("/api/v1/notifications")
@AllArgsConstructor 
@Validated 
public class NotificationController {
    private final NotificationService notificationService;  

    @PostMapping  
    @ResponseStatus(HttpStatus.CREATED)
    public NotificationResponse created(
        @Valid @RequestBody NotificationRequest request
    ){
        return notificationService.create(request);
    }

    @GetMapping 
    public List<NotificationResponse> findAll(
        @RequestParam(required = false) String status,
        @RequestParam(required = false) String type,
        @RequestParam (required = false) String search
    ){
        if(search != null && !search.isBlank()){
            return notificationService.searchByTitle(search);
        }
        return notificationService.filter(status, type);
    }


    @PutMapping("/{id}")
    public NotificationResponse update(
            @PathVariable @Positive Long id,
            @Valid @RequestBody NotificationRequest request) {

        return notificationService.update(id, request);
    }
}
