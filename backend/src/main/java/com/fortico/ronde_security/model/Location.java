package com.fortico.ronde_security.model;

import jakarta.persistence.Entity;
import jakarta.persistence.Id;
import jakarta.persistence.UniqueConstraint;
import lombok.Data;
import lombok.NoArgsConstructor;
import jakarta.persistence.Table;
import lombok.AllArgsConstructor;
import lombok.Builder;
import jakarta.persistence.Column;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.ManyToOne;
import jakarta.persistence.FetchType;
import jakarta.persistence.JoinColumn;
import jakarta.persistence.ForeignKey;

@Entity 
@Table(name="location", uniqueConstraints={
    @UniqueConstraint (name="uk_location_checkpoint", 
        columnNames="checkpoint_code"
    )
})
@Data 
@Builder
@AllArgsConstructor 
@NoArgsConstructor 
public class Location {
    @Id 
    @GeneratedValue (strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(
        name = "checkpoint_code",
        nullable = false,
        unique = true,
        length = 50
    )
    private String checkpointCode;

    @Column(
        name = "name",
        nullable = false,
        length = 100
    )
    private String name;

    @ManyToOne(fetch = FetchType.LAZY, optional = true)
    @JoinColumn(
        name = "agent_id",
        foreignKey = @ForeignKey (name = "fk_locations_agent")
    )
    private Agents agent;

    @Builder.Default
    @Column (
        name = "is_active",
        nullable = false
    )
    private Boolean isActive = true;

   
}
