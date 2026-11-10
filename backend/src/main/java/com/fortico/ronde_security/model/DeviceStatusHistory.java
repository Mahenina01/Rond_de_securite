package com.fortico.ronde_security.model;

import java.time.Instant;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.Index;
import jakarta.persistence.Table;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

@Entity
@Table(name = "device_status_history", indexes = {
    @Index(name = "idx_device_status_history_device_time", columnList = "device_id, changed_at")
})
@Getter
@Setter
@NoArgsConstructor
public class DeviceStatusHistory {

  @Id
  @GeneratedValue(strategy = GenerationType.IDENTITY)
  private Long id;

  @Column(name = "device_id", nullable = false)
  private Long deviceId;

  @Column(name = "is_active", nullable = false)
  private boolean active;

  @Column(name = "is_present", nullable = false)
  private boolean present;

  @Column(name = "is_baseline", nullable = false)
  private boolean baseline;

  @Column(name = "changed_at", nullable = false)
  private Instant changedAt;
}
