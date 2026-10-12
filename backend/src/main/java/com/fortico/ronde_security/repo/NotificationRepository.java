package com.fortico.ronde_security.repo;

import org.springframework.data.jpa.repository.JpaRepository;
import com.fortico.ronde_security.model.Notifications;
import java.util.List;

public interface NotificationRepository extends JpaRepository<Notifications, Long> {
    List<Notifications> findByStatus(String status);

    List<Notifications> findByType(String type);

    List<Notifications> findByStatusAndType(
            String status,
            String type);

    List<Notifications> findByTitle(String title);
}
