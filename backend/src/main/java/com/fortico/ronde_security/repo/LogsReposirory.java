package com.fortico.ronde_security.repo;

import java.time.Instant;
import java.util.List;

import org.springframework.data.jpa.repository.JpaRepository;

import com.fortico.ronde_security.enums.LogStatus;
import com.fortico.ronde_security.model.Logs;

public interface LogsReposirory extends JpaRepository<Logs, Long> {

  // Filtrer par status
  List<Logs> findByStatus(LogStatus status);

  // Filter par date
  List<Logs> findByTimesTamp(Instant timestamp);

  // Filtrer par agent
  List<Logs> findByAgents_AgentCode(String agent);

  // Filtrer par appareil
  List<Logs> findByDevices_DeviceCode(String deviceCode);

  // Filtrer par emplacement

}
