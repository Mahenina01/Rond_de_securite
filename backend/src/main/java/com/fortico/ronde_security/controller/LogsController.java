
package com.fortico.ronde_security.controller;

import java.time.Instant;
import java.time.LocalDate;
import java.util.List;

import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import com.fortico.ronde_security.dtos.logs.LogsRequest;
import com.fortico.ronde_security.dtos.logs.LogsResponse;
import com.fortico.ronde_security.enums.LogStatus;
import com.fortico.ronde_security.service.logs.LogService;

import jakarta.validation.Valid;

@RestController
@RequestMapping("/api/v1/logs")
public class LogsController {

  private final LogService logsService;

  public LogsController(LogService logsService) {
    this.logsService = logsService;
  }

  // POST /api/v1/logs
  @PostMapping
  public ResponseEntity<LogsResponse> createLog(
      @Valid @RequestBody LogsRequest request) {

    LogsResponse response = logsService.createLog(request);

    return ResponseEntity
        .status(HttpStatus.CREATED)
        .body(response);
  }

  // GET /api/v1/logs avec filtres optionnels
  @GetMapping
  public ResponseEntity<List<LogsResponse>> getLogs(
      @RequestParam(required = false) LogStatus status,
      @RequestParam(required = false) LocalDate date,
      @RequestParam(name = "agent_id", required = false) Long agentId,
      @RequestParam(name = "device_id", required = false) Long deviceId,
      @RequestParam(name = "location_id", required = false) Long locationId,
      @RequestParam(required = false) Instant from,
      @RequestParam(required = false) Instant to,
      @RequestParam(defaultValue = "timestamp") String sort,
      @RequestParam(defaultValue = "desc") String order) {

    List<LogsResponse> logs = logsService.getLogs(
        status,
        date,
        agentId,
        deviceId,
        locationId,
        from,
        to,
        sort,
        order);

    return ResponseEntity.ok(logs);
  }

  // GET /api/v1/logs/{id}
  @GetMapping("/{id}")
  public ResponseEntity<LogsResponse> getLogById(
      @PathVariable Long id) {

    return ResponseEntity.ok(logsService.getLogById(id));
  }
}
