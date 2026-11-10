
package com.fortico.ronde_security.repo;

import java.time.Instant;
import java.time.LocalDate;
import java.util.List;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.JpaSpecificationExecutor;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;

import com.fortico.ronde_security.enums.LogStatus;
import com.fortico.ronde_security.model.Logs;

public interface LogsRepository extends JpaRepository<Logs, Long>, JpaSpecificationExecutor<Logs> {

  interface DailyPassageCount {
    LocalDate getDay();

    long getTotal();
  }

  @Query(value = """
      SELECT CAST(l.timestamp AT TIME ZONE 'UTC' AS date) AS day,
             COUNT(*) AS total
      FROM logs l
      WHERE l.status = 'SUCCESS'
        AND l.timestamp >= :from
        AND l.timestamp < :to
      GROUP BY CAST(l.timestamp AT TIME ZONE 'UTC' AS date)
      ORDER BY day
      """, nativeQuery = true)
  List<DailyPassageCount> countSuccessfulPassagesByDay(
      @Param("from") Instant from,
      @Param("to") Instant to);

  long countByTimestampGreaterThanEqualAndTimestampLessThan(
      Instant from,
      Instant to);

  long countByStatusAndTimestampGreaterThanEqualAndTimestampLessThan(
      LogStatus status,
      Instant from,
      Instant to);

  // Filtrer par statut
  List<Logs> findByStatus(LogStatus status);

  // Filtrer par date et heure exacte du scan
  List<Logs> findByTimestamp(Instant timestamp);

  // Filtrer par agent
  List<Logs> findByAgent_AgentCode(String agentCode);

  void deleteByAgent_Id(Long agentId);

  void deleteByDevice_Agent_Id(Long agentId);

  void deleteByLocationId_Agent_Id(Long agentId);

  boolean existsByDevice_Id(Long deviceId);

  boolean existsByLocationId_Id(Long locationId);

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
