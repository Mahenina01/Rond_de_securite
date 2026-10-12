package com.fortico.ronde_security.model;

import jakarta.persistence.Entity;
import lombok.Builder;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.time.OffsetDateTime;

import jakarta.persistence.*;


@Entity 
@Table(name ="logo")
@Data 
@AllArgsConstructor 
@NoArgsConstructor 
@Builder
public class Logo {

    @Id 
    @GeneratedValue (strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(name = "photos", length = 500)
    private String photo;

    @Builder.Default
    @Column(name = "updated_at",nullable = false)
    private OffsetDateTime updatedAt = OffsetDateTime.now();

    @PreUpdate 
    public void preUpdate(){
        this.updatedAt = OffsetDateTime.now();
    }
}
