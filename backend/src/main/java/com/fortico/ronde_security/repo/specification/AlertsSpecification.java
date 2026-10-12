package com.fortico.ronde_security.repo.specification;

import java.time.Instant;

import org.springframework.data.jpa.domain.Specification;

import com.fortico.ronde_security.enums.AlertStatus;
import com.fortico.ronde_security.model.Alerts;

public final class AlertsSpecification {

  private AlertsSpecification() {
  }

  public static Specification<Alerts> hasStatus(AlertStatus status) {
    return (root, query, cb) -> status == null
        ? cb.conjunction()
        : cb.equal(root.get("status"), status);
  }

  public static Specification<Alerts> hasLocationId(Long locationId) {
    return (root, query, cb) -> locationId == null
        ? cb.conjunction()
        : cb.equal(
            root.get("locationId").get("id"),
            locationId);
  }

  public static Specification<Alerts> hasAgentId(Long agentId) {
    return (root, query, cb) -> agentId == null
        ? cb.conjunction()
        : cb.equal(
            root.get("locationId").get("agent").get("id"),
            agentId);
  }

  public static Specification<Alerts> receivedAfter(Instant from) {
    return (root, query, cb) -> from == null
        ? cb.conjunction()
        : cb.greaterThanOrEqualTo(
            root.get("receivedAt"),
            from);
  }

  public static Specification<Alerts> receivedBefore(Instant to) {
    return (root, query, cb) -> to == null
        ? cb.conjunction()
        : cb.lessThanOrEqualTo(
            root.get("receivedAt"),
            to);
  }
}