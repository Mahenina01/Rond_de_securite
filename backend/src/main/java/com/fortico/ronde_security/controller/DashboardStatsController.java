package com.fortico.ronde_security.controller;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.fortico.ronde_security.dtos.dashboard.DashboardStatsResponse;
import com.fortico.ronde_security.service.dashboard.DashboardStatsService;

@RestController
@RequestMapping("/api/v1/dashboard")
public class DashboardStatsController {

  private final DashboardStatsService dashboardStatsService;

  public DashboardStatsController(DashboardStatsService dashboardStatsService) {
    this.dashboardStatsService = dashboardStatsService;
  }

  @GetMapping("/stats")
  public DashboardStatsResponse getStats() {
    return dashboardStatsService.getStats();
  }
}
