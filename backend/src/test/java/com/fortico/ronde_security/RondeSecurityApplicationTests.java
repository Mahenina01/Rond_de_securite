package com.fortico.ronde_security;

import static org.junit.jupiter.api.Assertions.assertEquals;

import java.time.LocalDate;
import java.time.ZoneOffset;

import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;

import com.fortico.ronde_security.service.dashboard.WeeklyPassageService;
import com.fortico.ronde_security.service.dashboard.DashboardStatsService;

@SpringBootTest
class RondeSecurityApplicationTests {

	@Autowired
	private WeeklyPassageService weeklyPassageService;

	@Autowired
	private DashboardStatsService dashboardStatsService;

	@Test
	void contextLoads() {
	}

	@Test
	void weeklyPassagesIncludeSevenUtcDays() {
		var passages = weeklyPassageService.getWeeklyPassages();
		LocalDate today = LocalDate.now(ZoneOffset.UTC);

		assertEquals(7, passages.size());
		assertEquals(today.minusDays(6), passages.get(0).date());
		assertEquals(today, passages.get(6).date());
		assertEquals("Aujourd'hui", passages.get(6).day());
	}

	@Test
	void dashboardStatsAreCalculated() {
		var stats = dashboardStatsService.getStats();

		assertEquals(LocalDate.now(ZoneOffset.UTC), stats.date());
		org.junit.jupiter.api.Assertions.assertTrue(stats.confirmedRoundsToday() >= 0);
		org.junit.jupiter.api.Assertions.assertTrue(stats.activeDevices() >= 0);
		if (stats.successRatePercent() != null) {
			org.junit.jupiter.api.Assertions.assertTrue(
					stats.successRatePercent() >= 0 && stats.successRatePercent() <= 100);
		}
	}

}
