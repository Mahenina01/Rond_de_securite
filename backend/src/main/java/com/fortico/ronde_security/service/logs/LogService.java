
package com.fortico.ronde_security.service.logs;

import java.time.Instant;
import java.time.LocalDate;
import java.util.List;

import com.fortico.ronde_security.dtos.logs.LogsRequest;
import com.fortico.ronde_security.dtos.logs.LogsResponse;
import com.fortico.ronde_security.enums.LogStatus;

public interface LogService {

  LogsResponse createLog(LogsRequest request);

  List<LogsResponse> getLogs(
      LogStatus status,
      LocalDate date,
      Long agentId,
      Long deviceId,
      Long locationId,
      Instant from,
      Instant to,
      String sort,
      String order);

  LogsResponse getLogById(Long id);
}
