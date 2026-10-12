package com.fortico.ronde_security.model;

import java.time.Instant;

import com.fortico.ronde_security.enums.AlertStatus;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.EnumType;
import jakarta.persistence.Enumerated;
import jakarta.persistence.FetchType;
import jakarta.persistence.ForeignKey;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.JoinColumn;
import jakarta.persistence.ManyToOne;
import jakarta.persistence.PrePersist;
import jakarta.persistence.Table;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

@Entity
@Table(name = "alerts")
@Data
@NoArgsConstructor
@AllArgsConstructor
public class Alerts {

  @Id
  @GeneratedValue(strategy = GenerationType.IDENTITY)
  private Long id;

  @Column(name = "message", nullable = false, columnDefinition = "TEXT")
  private String message;

  @ManyToOne(fetch = FetchType.LAZY, optional = false)
  @JoinColumn(name = "location_id", referencedColumnName = "id", nullable = false)
  private Location locationId;

  @Enumerated(EnumType.STRING)
  @Column(name = "status", nullable = false, length = 30)
  private AlertStatus status;

  @Column(name = "received_at", nullable = false, updatable = false)
  private Instant receivedAt;

  @PrePersist
  protected void onCreate() {
    if (receivedAt == null) {
      receivedAt = Instant.now();
    }
  }
}