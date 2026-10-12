package com.fortico.ronde_security.service.alerts;

import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Service;

import com.fortico.ronde_security.dtos.alerts.AlertsResponse;

@Service
public class AlertsRealtimeService {

  private final SimpMessagingTemplate messagingTemplate;

  public AlertsRealtimeService(
      SimpMessagingTemplate messagingTemplate) {
    this.messagingTemplate = messagingTemplate;
  }

  public void publishAlert(AlertsResponse alert) {
    messagingTemplate.convertAndSend(
        "/topic/alerts",
        alert);
  }
}