package com.fortico.ronde_security.repo;

import java.time.Instant;
import java.util.List;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.JpaSpecificationExecutor;

import com.fortico.ronde_security.enums.AlertStatus;
import com.fortico.ronde_security.model.Alerts;

public interface AlertsRepository
        extends JpaRepository<Alerts, Long>,
        JpaSpecificationExecutor<Alerts> {

    List<Alerts> findAllByOrderByReceivedAtDesc();

    List<Alerts> findByStatusOrderByReceivedAtDesc(
            AlertStatus status);

    List<Alerts> findByLocationId_IdOrderByReceivedAtDesc(
            Long locationId);

    List<Alerts> findByLocationId_Agent_IdOrderByReceivedAtDesc(
            Long agentId);

    List<Alerts> findByReceivedAtBetweenOrderByReceivedAtDesc(
            Instant from,
            Instant to);
}