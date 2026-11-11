package com.fortico.ronde_security.service.notifications;

import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Service;

import com.fortico.ronde_security.dtos.notification.NotificationResponse;

@Service
public class NotificationRealtimeService {

  private static final String NOTIFICATIONS_TOPIC = "/topic/notifications";

  private final SimpMessagingTemplate messagingTemplate;

  public NotificationRealtimeService(SimpMessagingTemplate messagingTemplate) {
    this.messagingTemplate = messagingTemplate;
  }

  public void publish(NotificationResponse notification) {
    messagingTemplate.convertAndSend(NOTIFICATIONS_TOPIC, notification);
  }
}
