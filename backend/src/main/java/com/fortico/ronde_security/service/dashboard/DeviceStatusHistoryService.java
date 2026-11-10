package com.fortico.ronde_security.service.dashboard;

import java.time.Instant;

import org.springframework.context.ApplicationEventPublisher;
import org.springframework.context.event.EventListener;
import org.springframework.boot.context.event.ApplicationReadyEvent;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import com.fortico.ronde_security.model.DeviceStatusHistory;
import com.fortico.ronde_security.model.Devices;
import com.fortico.ronde_security.repo.DeviceStatusHistoryRepository;
import com.fortico.ronde_security.repo.DevicesRepository;

@Service
public class DeviceStatusHistoryService {

  private final DeviceStatusHistoryRepository historyRepository;
  private final DevicesRepository devicesRepository;
  private final ApplicationEventPublisher eventPublisher;

  public DeviceStatusHistoryService(
      DeviceStatusHistoryRepository historyRepository,
      DevicesRepository devicesRepository,
      ApplicationEventPublisher eventPublisher) {
    this.historyRepository = historyRepository;
    this.devicesRepository = devicesRepository;
    this.eventPublisher = eventPublisher;
  }

  @EventListener(ApplicationReadyEvent.class)
  @Transactional
  public void initializeLegacyDeviceBaseline(ApplicationReadyEvent event) {
    historyRepository.insertMissingDeviceBaselines(Instant.now());
  }

  @Transactional
  public void recordCreated(Devices device) {
    recordState(device.getId(), device.getIsActive(), true, false);
    publishStatsChange();
  }

  @Transactional
  public void recordStatusChange(Devices device) {
    recordState(device.getId(), device.getIsActive(), true, false);
    publishStatsChange();
  }

  @Transactional
  public void recordDeleted(Devices device) {
    recordState(device.getId(), false, false, false);
    publishStatsChange();
  }

  @Transactional
  public void recordAgentDevicesDeleted(Long agentId) {
    devicesRepository.findByAgent_Id(agentId)
        .forEach(device -> recordState(device.getId(), false, false, false));
    publishStatsChange();
  }

  private void recordState(
      Long deviceId,
      boolean active,
      boolean present,
      boolean baseline) {
    DeviceStatusHistory history = new DeviceStatusHistory();
    history.setDeviceId(deviceId);
    history.setActive(active);
    history.setPresent(present);
    history.setBaseline(baseline);
    history.setChangedAt(Instant.now());
    historyRepository.save(history);
  }

  private void publishStatsChange() {
    eventPublisher.publishEvent(new DashboardStatsChangedEvent());
  }
}
