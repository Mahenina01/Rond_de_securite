package com.fortico.ronde_security.model;

import java.time.OffsetDateTime;

import jakarta.persistence.*;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.*;
import lombok.NoArgsConstructor;
import lombok.Setter;
import jakarta.persistence.Table;
import jakarta.persistence.Id;
import jakarta.persistence.GeneratedValue;


@Entity 
@Table(name = "notifications")
@Getter 
@Setter 
@NoArgsConstructor 
@AllArgsConstructor 
@Builder 
public class Notifications {
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
