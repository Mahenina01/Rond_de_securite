package com.fortico.ronde_security.controller;

import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseStatus;
import org.springframework.web.bind.annotation.RestController;

import com.fortico.ronde_security.dtos.devices.DeviceProvisioningResponse;
import com.fortico.ronde_security.dtos.devices.DevicesRequest;
import com.fortico.ronde_security.dtos.devices.DevicesResponse;
import com.fortico.ronde_security.service.devices.DevicesService;

import jakarta.validation.Valid;
import lombok.AllArgsConstructor;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;

import java.util.List;

import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PatchMapping;
import org.springframework.web.bind.annotation.PathVariable;

@RestController
@RequestMapping("/api/v1/devices")
@AllArgsConstructor
public class DevicesController {

  private final DevicesService devicesServices;

  // create device
  @PostMapping()
  @ResponseStatus(org.springframework.http.HttpStatus.CREATED)
  public DeviceProvisioningResponse create(@Valid @RequestBody DevicesRequest request) {

    return devicesServices.createDevice(request);
  }

  // get all devices
  @GetMapping()
  public List<DevicesResponse> getAll() {
    return devicesServices.getAllDevices();
  }

  // get device by id
  @GetMapping("/{id}")
  public DevicesResponse getById(@PathVariable Long id) {
    return devicesServices.getDeviceById(id);
  }

  // get device by code
  @GetMapping("/code/{deviceCode}")
  public DevicesResponse getByCode(@PathVariable String deviceCode) {
    return devicesServices.getDeviceByCode(deviceCode);
  }

  // get devices by agent id
  @GetMapping("/agent/{agentCode}")
  public List<DevicesResponse> getByAgentId(@PathVariable String agentCode) {
    return devicesServices.getDevicesByAgentCode(agentCode);
  }

  // update device
  @PatchMapping("/update/{id}")
  public DevicesResponse update(@PathVariable Long id, @RequestBody DevicesRequest request) {
    return devicesServices.updateDevice(id, request);
  }

  // update status
  @PatchMapping("/update/status/{id}")
  public DevicesResponse updateStatus(@PathVariable Long id, @RequestParam Boolean status) {
    return devicesServices.updateDeviceStatus(id, status);
  }

  // delede
  @DeleteMapping("/delete/{id}")
  public void delete(@PathVariable Long id) {
    devicesServices.deleteDevice(id);
  }
}
