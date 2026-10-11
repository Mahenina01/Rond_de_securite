package com.fortico.ronde_security.model;

import java.time.OffsetDateTime;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.Id;
import jakarta.persistence.PrePersist;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;
@Getter 
@Setter 
@AllArgsConstructor 
@NoArgsConstructor 
@Entity(name = "agents")
@Data
@Builder 
public class Agents {
    @Id
    @GeneratedValue(strategy = jakarta.persistence.GenerationType.IDENTITY)
    private Long id;
    @Column(name = "agent_code", nullable = false, unique = true, length = 100)
    private String agent_code;
    @Column(name = "name", nullable = false, length = 100)
    private String name;
    @Column(name = "phone", nullable = false, length = 15)
    private String phone;
    @Column(name = "is_active", nullable = false)
    private Boolean is_active;
    @Builder.Default
    @Column(name = "created_at", nullable = false)
    private OffsetDateTime created_at = OffsetDateTime.now();
    @PrePersist 
    private void prePersist() {
        if(created_at== null) {
            created_at = OffsetDateTime.now();
        }
        if (is_active == null) {
            is_active = true;
        }
    }
}
