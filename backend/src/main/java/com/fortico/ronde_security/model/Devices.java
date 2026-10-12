package com.fortico.ronde_security.model;

import jakarta.persistence.*;
import lombok.Data;
import lombok.NoArgsConstructor;
import lombok.AllArgsConstructor;
import lombok.Builder;

@Entity
@Table(name = "devices")
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class Devices {

  @Id
  @GeneratedValue(strategy = GenerationType.IDENTITY)
  private Long id;

  @Column(name = "device_code", length = 50, nullable = false, unique = true)
  private String deviceCode;

  @Column(name = "key_hash", length = 64, nullable = false)
  private String keyHash;

  @Column(name = "key_encrypted", columnDefinition = "TEXT")
  private String keyEncrypted;

  @ManyToOne(fetch = FetchType.LAZY, optional = false)
  @JoinColumn(name = "agent_id", nullable = false)
  private Agents agent;

  @Column(name = "is_active", nullable = false)
  @Builder.Default
  private Boolean isActive = true;
}