package com.fortico.ronde_security.dtos.notification;
import java.time.OffsetDateTime;
public record NotificationResponse(

    Long id,
    String title,
    String description,
    OffsetDateTime timestamp,
    String type,
    String status
) {
    
}
