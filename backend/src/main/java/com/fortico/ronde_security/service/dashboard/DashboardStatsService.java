package com.fortico.ronde_security.service.dashboard;

import java.time.Instant;
import java.time.LocalDate;
import java.time.ZoneOffset;

import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.transaction.event.TransactionPhase;
import org.springframework.transaction.event.TransactionalEventListener;

import com.fortico.ronde_security.dtos.dashboard.DashboardStatsResponse;
import com.fortico.ronde_security.enums.LogStatus;
import com.fortico.ronde_security.repo.DeviceStatusHistoryRepository;
import com.fortico.ronde_security.repo.DevicesRepository;
import com.fortico.ronde_security.repo.LogsRepository;

@Service
public class DashboardStatsService {

  private static final String DASHBOARD_STATS_TOPIC =
      "/topic/dashboard/stats";

  private final LogsRepository logsRepository;
  private final DevicesRepository devicesRepository;
  private final DeviceStatusHistoryRepository deviceStatusHistoryRepository;
  private final SimpMessagingTemplate messagingTemplate;

  public DashboardStatsService(
      LogsRepository logsRepository,
      DevicesRepository devicesRepository,
      DeviceStatusHistoryRepository deviceStatusHistoryRepository,
      SimpMessagingTemplate messagingTemplate) {
    this.logsRepository = logsRepository;
    this.devicesRepository = devicesRepository;
    this.deviceStatusHistoryRepository = deviceStatusHistoryRepository;
    this.messagingTemplate = messagingTemplate;
  }

  @Transactional(readOnly = true)
  public DashboardStatsResponse getStats() {
    LocalDate today = LocalDate.now(ZoneOffset.UTC);
    Instant todayStart = today.atStartOfDay(ZoneOffset.UTC).toInstant();
    Instant tomorrowStart = today.plusDays(1).atStartOfDay(ZoneOffset.UTC).toInstant();
    Instant yesterdayStart = today.minusDays(1).atStartOfDay(ZoneOffset.UTC).toInstant();

    long confirmedToday = countLogs(LogStatus.SUCCESS, todayStart, tomorrowStart);
    long confirmedYesterday = countLogs(LogStatus.SUCCESS, yesterdayStart, todayStart);
    long totalToday = countLogs(null, todayStart, tomorrowStart);
    long totalYesterday = countLogs(null, yesterdayStart, todayStart);

    long activeDevices = devicesRepository.countByIsActiveTrue();
    boolean deviceHistoryComplete = !deviceStatusHistoryRepository
        .existsByBaselineTrueAndChangedAtAfter(yesterdayStart);
    Long activeDevicesYesterday = deviceHistoryComplete
        ? deviceStatusHistoryRepository.countActiveDevicesAt(todayStart)
        : null;

    Double successRateToday = percentage(confirmedToday, totalToday);
    Double successRateYesterday = percentage(confirmedYesterday, totalYesterday);

    return new DashboardStatsResponse(
        today,
        confirmedToday,
        relativeChange(confirmedToday, confirmedYesterday),
        activeDevices,
        activeDevicesYesterday == null
            ? null
            : relativeChange(activeDevices, activeDevicesYesterday),
        successRateToday,
        successRateToday == null || successRateYesterday == null
            ? null
            : relativeChange(successRateToday, successRateYesterday));
  }

  @TransactionalEventListener(phase = TransactionPhase.AFTER_COMMIT)
  public void publishAfterDashboardStatsChange(DashboardStatsChangedEvent event) {
    messagingTemplate.convertAndSend(DASHBOARD_STATS_TOPIC, getStats());
  }

  private long countLogs(LogStatus status, Instant from, Instant to) {
    return status == null
        ? logsRepository.countByTimestampGreaterThanEqualAndTimestampLessThan(from, to)
        : logsRepository.countByStatusAndTimestampGreaterThanEqualAndTimestampLessThan(
            status, from, to);
  }

  private Double percentage(long numerator, long denominator) {
    return denominator == 0
        ? null
        : numerator * 100.0 / denominator;
  }

  private Double relativeChange(double today, double yesterday) {
    if (yesterday == 0) {
      return today == 0 ? 0.0 : null;
    }
    return (today - yesterday) * 100.0 / yesterday;
  }
}
