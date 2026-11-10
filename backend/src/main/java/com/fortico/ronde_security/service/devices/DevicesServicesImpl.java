
package com.fortico.ronde_security.service.devices;

import java.util.List;

import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import com.fortico.ronde_security.dtos.devices.DeviceProvisioningResponse;
import com.fortico.ronde_security.dtos.devices.DevicesRequest;
import com.fortico.ronde_security.dtos.devices.DevicesResponse;
import com.fortico.ronde_security.exception.DuplicateResourceException;
import com.fortico.ronde_security.exception.ResourceInUseException;
import com.fortico.ronde_security.exception.ResourceNotFoundException;
import com.fortico.ronde_security.model.Agents;
import com.fortico.ronde_security.model.Devices;
import com.fortico.ronde_security.repo.AgentRepository;
import com.fortico.ronde_security.repo.DevicesRepository;
import com.fortico.ronde_security.repo.LogsRepository;
import com.fortico.ronde_security.service.dashboard.DeviceStatusHistoryService;
import com.fortico.ronde_security.utils.DeviceCryptoUtils;

@Service
@Transactional
public class DevicesServicesImpl implements DevicesService {

  private final DevicesRepository devicesRepository;
  private final AgentRepository agentRepository;
  private final LogsRepository logsRepository;
  private final DeviceStatusHistoryService deviceStatusHistoryService;
  private final DeviceCryptoUtils deviceCryptoUtils;

  public DevicesServicesImpl(
      DevicesRepository devicesRepository,
      AgentRepository agentRepository,
      LogsRepository logsRepository,
      DeviceStatusHistoryService deviceStatusHistoryService,
      DeviceCryptoUtils deviceCryptoUtils) {

    this.devicesRepository = devicesRepository;
    this.agentRepository = agentRepository;
    this.logsRepository = logsRepository;
    this.deviceStatusHistoryService = deviceStatusHistoryService;
    this.deviceCryptoUtils = deviceCryptoUtils;
  }

  // Création d'un nouvel appareil

  @Override
  public DeviceProvisioningResponse createDevice(DevicesRequest request) {

    if (devicesRepository.existsByDeviceCode(request.deviceCode())) {
      throw new DuplicateResourceException(
          "Un appareil utilise déjà ce code.");
    }

    Agents agent = agentRepository.findByName(request.nameAgent())
        .orElseThrow(() -> new ResourceNotFoundException(
            "Agent non trouvé."));

    String secret = deviceCryptoUtils.generateSecret();

    Devices device = new Devices();
    device.setDeviceCode(request.deviceCode());
    device.setKeyHash(deviceCryptoUtils.sha256Hex(secret));
    device.setKeyEncrypted(deviceCryptoUtils.encrypt(secret));
    device.setAgent(agent);
    device.setIsActive(true);

    Devices saved = devicesRepository.save(device);
    deviceStatusHistoryService.recordCreated(saved);

    return new DeviceProvisioningResponse(
        saved.getId(),
        saved.getDeviceCode(),
        saved.getAgent().getAgentCode(),
        saved.getIsActive(),
        secret);
  }

  // Récupération de tous les appareils
  @Override
  @Transactional(readOnly = true)
  public List<DevicesResponse> getAllDevices() {
    return devicesRepository.findAll()
        .stream()
        .map(this::toDevicesResponse)
        .toList();
  }

  // Récupération par identifiant
  @Override
  @Transactional(readOnly = true)
  public DevicesResponse getDeviceById(Long id) {
    Devices device = findDeviceOrThrow(id);
    return toDevicesResponse(device);
  }

  // Mise à jour d'un appareil
  @Override
  public DevicesResponse updateDevice(
      Long id, DevicesRequest request) {

    Devices device = findDeviceOrThrow(id);

    if (devicesRepository.existsByDeviceCodeAndIdNot(
        request.deviceCode(), id)) {
      throw new DuplicateResourceException(
          "Un autre appareil utilise déjà ce code.");
    }

    Agents agent = agentRepository.findByName(request.nameAgent())
        .orElseThrow(() -> new ResourceNotFoundException(
            "Agent non trouvé."));

    device.setDeviceCode(request.deviceCode());
    device.setAgent(agent);

    return toDevicesResponse(devicesRepository.save(device));
  }

  // Suppression d'un appareil
  @Override
  public void deleteDevice(Long id) {
    Devices device = findDeviceOrThrow(id);
    if (logsRepository.existsByDevice_Id(id)) {
      throw new ResourceInUseException(
          "Impossible de supprimer cet appareil : des logs y sont associés.");
    }
    deviceStatusHistoryService.recordDeleted(device);
    devicesRepository.delete(device);
  }

  // Mise à jour du statut
  @Override
  public DevicesResponse updateDeviceStatus(
      Long id, Boolean isActive) {

    if (isActive == null) {
      throw new IllegalArgumentException(
          "Le statut ne peut pas être null.");
    }

    Devices device = findDeviceOrThrow(id);
    if (!isActive.equals(device.getIsActive())) {
      device.setIsActive(isActive);
      Devices updatedDevice = devicesRepository.save(device);
      deviceStatusHistoryService.recordStatusChange(updatedDevice);
      return toDevicesResponse(updatedDevice);
    }

    return toDevicesResponse(device);
  }

  // Récupération par code
  @Override
  @Transactional(readOnly = true)
  public DevicesResponse getDeviceByCode(String deviceCode) {
    Devices device = devicesRepository.findByDeviceCode(deviceCode)
        .orElseThrow(() -> new ResourceNotFoundException(
            "Appareil non trouvé."));

    return toDevicesResponse(device);
  }

  // Récupération des appareils d'un agent
  @Override
  @Transactional(readOnly = true)
  public List<DevicesResponse> getDevicesByAgentCode(String agentCode) {
    return devicesRepository.findByAgent_AgentCode(agentCode)
        .stream()
        .map(this::toDevicesResponse)
        .toList();
  }

  private Devices findDeviceOrThrow(Long id) {
    return devicesRepository.findById(id)
        .orElseThrow(() -> new ResourceNotFoundException(
            "Appareil non trouvé."));
  }

  private DevicesResponse toDevicesResponse(Devices device) {
    return new DevicesResponse(
        device.getId(),
        device.getDeviceCode(),
        device.getAgent().getAgentCode(),
        device.getIsActive());
  }
}
