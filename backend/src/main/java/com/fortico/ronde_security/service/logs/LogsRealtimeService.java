
package com.fortico.ronde_security.service.logs;

import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Service;

import com.fortico.ronde_security.dtos.logs.LogsResponse;

@Service
public class LogsRealtimeService {

  private static final String LOGS_TOPIC = "/topic/logs";

  private final SimpMessagingTemplate messagingTemplate;

  public LogsRealtimeService(
      SimpMessagingTemplate messagingTemplate) {
    this.messagingTemplate = messagingTemplate;
  }

  public void publishLog(LogsResponse log) {
    messagingTemplate.convertAndSend(LOGS_TOPIC, log);
  }
}
