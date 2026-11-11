package com.fortico.ronde_security.dtos.notification;

import jakarta.validation.constraints.NotNull;

public record NotificationReadRequest(@NotNull Boolean read) {
}
