package com.fortico.ronde_security.service.notifications;

import com.fortico.ronde_security.dtos.notification.NotificationRequest;
import com.fortico.ronde_security.dtos.notification.NotificationResponse;
import java.util.List;

public interface NotificationService {
    NotificationResponse create(NotificationRequest request);

    NotificationResponse createFromMqtt(
        String title,
        String description,
        String type);

    NotificationResponse createRejectedMqtt(
        String title,
        String description,
        String type);

    List<NotificationResponse> findAll();
    NotificationResponse findById(Long id);

    List<NotificationResponse> filter(
        String status,
        String type
    );
    NotificationResponse update(
            Long id,
            NotificationRequest request);
    
    List<NotificationResponse> searchByTitle(String title);
    NotificationResponse archive(Long id) ;

    NotificationResponse updateReadStatus(Long id, boolean read);
}
