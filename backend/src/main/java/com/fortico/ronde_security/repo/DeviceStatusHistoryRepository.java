package com.fortico.ronde_security.repo;

import java.time.Instant;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Modifying;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;

import com.fortico.ronde_security.model.DeviceStatusHistory;

public interface DeviceStatusHistoryRepository
    extends JpaRepository<DeviceStatusHistory, Long> {

  boolean existsByDeviceId(Long deviceId);

  boolean existsByBaselineTrueAndChangedAtAfter(Instant instant);

  @Query(value = """
      SELECT COUNT(*)
      FROM (
        SELECT DISTINCT ON (device_id)
               device_id, is_active, is_present
        FROM device_status_history
        WHERE changed_at < :asOf
        ORDER BY device_id, changed_at DESC, id DESC
      ) latest
      WHERE is_active = TRUE AND is_present = TRUE
      """, nativeQuery = true)
  long countActiveDevicesAt(@Param("asOf") Instant asOf);

  @Modifying
  @Query(value = """
      INSERT INTO device_status_history
          (device_id, is_active, is_present, is_baseline, changed_at)
      SELECT d.id, d.is_active, TRUE, TRUE, :now
      FROM devices d
      WHERE NOT EXISTS (
          SELECT 1
          FROM device_status_history h
          WHERE h.device_id = d.id
      )
      """, nativeQuery = true)
  int insertMissingDeviceBaselines(@Param("now") Instant now);
}
