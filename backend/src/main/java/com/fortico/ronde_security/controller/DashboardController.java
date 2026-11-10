package com.fortico.ronde_security.controller;

import java.util.List;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.fortico.ronde_security.dtos.dashboard.WeeklyPassageResponse;
import com.fortico.ronde_security.service.dashboard.WeeklyPassageService;

@RestController
@RequestMapping("/api/v1/dashboard")
public class DashboardController {

  private final WeeklyPassageService weeklyPassageService;

  public DashboardController(WeeklyPassageService weeklyPassageService) {
    this.weeklyPassageService = weeklyPassageService;
  }

  @GetMapping("/weekly-passages")
  public List<WeeklyPassageResponse> getWeeklyPassages() {
    return weeklyPassageService.getWeeklyPassages();
  }
}
