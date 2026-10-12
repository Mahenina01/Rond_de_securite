package com.fortico.ronde_security.repo;

import org.springframework.data.jpa.repository.JpaRepository;
import com.fortico.ronde_security.model.Notification;
import java.util.List;

public interface NotificationRepository extends JpaRepository<Notification, Long> {

    List<Notification> findByStatus(String status);

    List<Notification> findByType(String type);

    List<Notification> findByStatusAndType(
            String status,
            String type);

    List<Notification> findByTitle(String title);
}
