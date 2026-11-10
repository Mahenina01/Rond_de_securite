package com.fortico.ronde_security.service.dashboard;

import java.time.Instant;
import java.time.LocalDate;
import java.time.ZoneOffset;
import java.time.format.TextStyle;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Service;
import org.springframework.transaction.event.TransactionalEventListener;
import org.springframework.transaction.event.TransactionPhase;
import org.springframework.transaction.annotation.Transactional;

import com.fortico.ronde_security.dtos.dashboard.WeeklyPassageResponse;
import com.fortico.ronde_security.repo.LogsRepository;
import com.fortico.ronde_security.service.logs.SuccessfulLogRecordedEvent;

@Service
public class WeeklyPassageService {

  private static final String WEEKLY_PASSAGES_TOPIC =
      "/topic/dashboard/weekly-passages";

  private final LogsRepository logsRepository;
  private final SimpMessagingTemplate messagingTemplate;

  public WeeklyPassageService(
      LogsRepository logsRepository,
      SimpMessagingTemplate messagingTemplate) {
    this.logsRepository = logsRepository;
    this.messagingTemplate = messagingTemplate;
  }

  @Transactional(readOnly = true)
  public List<WeeklyPassageResponse> getWeeklyPassages() {
    LocalDate today = LocalDate.now(ZoneOffset.UTC);
    LocalDate firstDay = today.minusDays(6);
    Instant from = firstDay.atStartOfDay(ZoneOffset.UTC).toInstant();
    Instant to = today.plusDays(1).atStartOfDay(ZoneOffset.UTC).toInstant();

    Map<LocalDate, Long> totalsByDay = logsRepository
        .countSuccessfulPassagesByDay(from, to)
        .stream()
        .collect(Collectors.toMap(
            LogsRepository.DailyPassageCount::getDay,
            LogsRepository.DailyPassageCount::getTotal));

    return Stream.iterate(firstDay, day -> day.plusDays(1))
        .limit(7)
        .map(day -> new WeeklyPassageResponse(
            day,
            formatDay(day, today),
            totalsByDay.getOrDefault(day, 0L)))
        .toList();
  }

  @TransactionalEventListener(phase = TransactionPhase.AFTER_COMMIT)
  public void publishAfterSuccessfulLog(SuccessfulLogRecordedEvent event) {
    messagingTemplate.convertAndSend(
        WEEKLY_PASSAGES_TOPIC,
        getWeeklyPassages());
  }

  private String formatDay(LocalDate day, LocalDate today) {
    if (day.equals(today)) {
      return "Aujourd'hui";
    }

    String shortDay = day.getDayOfWeek()
        .getDisplayName(TextStyle.SHORT, Locale.FRENCH)
        .replace(".", "");
    return Character.toUpperCase(shortDay.charAt(0))
        + shortDay.substring(1);
  }
}
