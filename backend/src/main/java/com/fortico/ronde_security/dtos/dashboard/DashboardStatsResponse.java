package com.fortico.ronde_security.dtos.dashboard;

import java.time.LocalDate;

public record DashboardStatsResponse(
    LocalDate date,
    long confirmedRoundsToday,
    Double roundsChangePercent,
    long activeDevices,
    Double activeDevicesChangePercent,
    Double successRatePercent,
    Double successRateChangePercent) {
}
