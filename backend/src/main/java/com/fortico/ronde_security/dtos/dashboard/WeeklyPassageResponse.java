package com.fortico.ronde_security.dtos.dashboard;

import java.time.LocalDate;

public record WeeklyPassageResponse(
    LocalDate date,
    String day,
    long value) {
}
