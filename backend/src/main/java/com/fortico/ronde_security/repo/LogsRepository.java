
package com.fortico.ronde_security.repo;

import java.time.Instant;
import java.util.List;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.JpaSpecificationExecutor;

import com.fortico.ronde_security.enums.LogStatus;
import com.fortico.ronde_security.model.Logs;

public interface LogsRepository extends JpaRepository<Logs, Long>, JpaSpecificationExecutor<Logs> {

  // Filtrer par statut
  List<Logs> findByStatus(LogStatus status);

  // Filtrer par date et heure exacte du scan
  List<Logs> findByTimestamp(Instant timestamp);

  // Filtrer par agent
  List<Logs> findByAgent_AgentCode(String agentCode);

  // Filtrer par appareil
  List<Logs> findByDevice_DeviceCode(String deviceCode);

  // Filtrer par emplacement
  List<Logs> findByLocationId_Id(Long locationId);

  // Filtrer par période : entre deux dates incluses
  List<Logs> findByTimestampBetween(
      Instant start,
      Instant end);

  // Filtrer les logs depuis une date
  List<Logs> findByTimestampGreaterThanEqual(Instant start);

  // Filtrer les logs jusqu'à une date
  List<Logs> findByTimestampLessThanEqual(Instant end);

  // Trier par date du scan : plus récent en premier
  List<Logs> findAllByOrderByTimestampDesc();

  // Trier par date du scan : plus ancien en premier
  List<Logs> findAllByOrderByTimestampAsc();

  // Trier par code agent A-Z
  List<Logs> findAllByOrderByAgent_AgentCodeAsc();

  List<Logs> findAllByOrderByDevice_DeviceCodeAsc();

  List<Logs> findAllByOrderByStatusAsc();
}
