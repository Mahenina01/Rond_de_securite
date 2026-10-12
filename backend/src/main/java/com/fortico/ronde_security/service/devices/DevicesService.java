package com.fortico.ronde_security.service.devices;

import java.util.List;

import com.fortico.ronde_security.dtos.devices.DeviceProvisioningResponse;
import com.fortico.ronde_security.dtos.devices.DevicesRequest;
import com.fortico.ronde_security.dtos.devices.DevicesResponse;
import com.fortico.ronde_security.model.Agents;

public interface DevicesService {
  // creation nouveau appareil
  DeviceProvisioningResponse createDevice(DevicesRequest request);

  // recuperation de tous les appareils
  List<DevicesResponse> getAllDevices();

  // recuperation d'un appareil par son id
  DevicesResponse getDeviceById(Long id);

  // mise a jour d'un appareil
  DevicesResponse updateDevice(Long id, DevicesRequest request);

  // suppression d'un appareil
  void deleteDevice(Long id);

  // mise a jour du statut d'un appareil
  DevicesResponse updateDeviceStatus(Long id, Boolean isActive);

  // recuperation d'un appareil par son code
  DevicesResponse getDeviceByCode(String deviceCode);

  // recuperation de tous les appareils d'un agent
  List<DevicesResponse> getDevicesByAgentCode(String agentId);
}
