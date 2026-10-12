
package com.fortico.ronde_security.model;

import java.time.Instant;
import java.util.UUID;

import com.fortico.ronde_security.enums.LogStatus;
import jakarta.persistence.*;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Entity
@Table(name = "logs", uniqueConstraints = {
    @UniqueConstraint(name = "uk_logs_event_uuid", columnNames = "event_uuid")
})
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class Logs {

  @Id
  @GeneratedValue(strategy = GenerationType.IDENTITY)
  private Long id;

  @Column(name = "event_uuid", nullable = false, unique = true, updatable = false)
  @Builder.Default
  private UUID eventUuid = UUID.randomUUID();

  @ManyToOne(fetch = FetchType.LAZY, optional = false)
  @JoinColumn(name = "agent_code", referencedColumnName = "agent_code", nullable = false)
  private Agents agent;

  @ManyToOne(fetch = FetchType.LAZY, optional = false)
  @JoinColumn(name = "device_code", referencedColumnName = "device_code", nullable = false)
  private Devices device;

  @Enumerated(EnumType.STRING)
  @Column(name = "status", length = 30, nullable = false)
  private LogStatus status;

  @Column(name = "timestamp", nullable = false)
  private Instant timestamp;

  @Column(name = "received_at", nullable = false, updatable = false)
  private Instant receivedAt;

  @PrePersist
  protected void onCreate() {
    if (eventUuid == null) {
      eventUuid = UUID.randomUUID();
    }

    if (receivedAt == null) {
      receivedAt = Instant.now();
    }
  }
}
