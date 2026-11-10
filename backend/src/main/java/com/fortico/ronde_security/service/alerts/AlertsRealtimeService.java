package com.fortico.ronde_security.service.alerts;

import org.springframework.context.ApplicationEventPublisher;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Service;
import org.springframework.transaction.event.TransactionPhase;
import org.springframework.transaction.event.TransactionalEventListener;

import com.fortico.ronde_security.dtos.alerts.AlertRealtimeEvent;
import com.fortico.ronde_security.dtos.alerts.AlertsResponse;

@Service
public class AlertsRealtimeService {

  private final SimpMessagingTemplate messagingTemplate;
  private final ApplicationEventPublisher eventPublisher;

  public AlertsRealtimeService(
      SimpMessagingTemplate messagingTemplate,
      ApplicationEventPublisher eventPublisher) {
    this.messagingTemplate = messagingTemplate;
    this.eventPublisher = eventPublisher;
  }

  public void publishAlert(AlertsResponse alert) {
    eventPublisher.publishEvent(new AlertRealtimeEvent(
        AlertRealtimeEvent.Action.UPSERT,
        alert.id(),
        alert));
  }

  public void publishDeleted(Long alertId) {
    eventPublisher.publishEvent(new AlertRealtimeEvent(
        AlertRealtimeEvent.Action.DELETE,
        alertId,
        null));
  }

  @TransactionalEventListener(phase = TransactionPhase.AFTER_COMMIT)
  public void publishAfterCommit(AlertRealtimeEvent event) {
    messagingTemplate.convertAndSend(
        "/topic/alerts",
        event);
  }
}