package com.fortico.ronde_security.model;

import java.time.OffsetDateTime;

import jakarta.persistence.*;

import lombok.*;


@Entity 
@Table(name = "notifications")
@Data
@NoArgsConstructor 
@AllArgsConstructor 
@Builder 
public class Notification {
    @Id 
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(length =200)
    private String title;

    @Column(columnDefinition = "Text")
    private String description;

    @Builder.Default
    @Column(name = "timestamp")
    private OffsetDateTime timestamp = OffsetDateTime.now();

    @Column(length = 50)
    private String type;

    @Builder.Default
    @Column(length = 30)
    private String status = "ACTIVE";

    


}
