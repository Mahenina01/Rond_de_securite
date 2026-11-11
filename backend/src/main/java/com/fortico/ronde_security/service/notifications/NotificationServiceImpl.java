package com.fortico.ronde_security.service.notifications;

import com.fortico.ronde_security.dtos.notification.NotificationRequest;
import com.fortico.ronde_security.dtos.notification.NotificationResponse;
import com.fortico.ronde_security.repo.NotificationRepository;

import org.springframework.stereotype.Service;

import com.fortico.ronde_security.model.Notification;
import java.util.List;

import org.springframework.transaction.annotation.*;
import org.springframework.transaction.annotation.Propagation;
import org.springframework.transaction.support.TransactionSynchronization;
import org.springframework.transaction.support.TransactionSynchronizationManager;
import org.springframework.http.HttpStatus;
import org.springframework.web.server.ResponseStatusException;

@Service
public class NotificationServiceImpl implements NotificationService {
        private final NotificationRepository notificationRepository;
        private final NotificationRealtimeService notificationRealtimeService;

        public NotificationServiceImpl(
                        NotificationRepository notificationRepository,
                        NotificationRealtimeService notificationRealtimeService) {
                this.notificationRepository = notificationRepository;
                this.notificationRealtimeService = notificationRealtimeService;
        }

        @Override
        public NotificationResponse create(
                        NotificationRequest request) {
                Notification notification = Notification.builder()
                                .title(request.title().trim())
                                .description(request.description().trim())
                                .type(request.type().trim())
                                .status("ACTIVE")
                                .build();

                Notification saved = notificationRepository.save(notification);

                return toResponse(saved);

        }

        @Override
        @Transactional
        public NotificationResponse createFromMqtt(
                        String title,
                        String description,
                        String type) {
                return saveMqttNotification(title, description, type);
        }

        @Override
        @Transactional(propagation = Propagation.REQUIRES_NEW)
        public NotificationResponse createRejectedMqtt(
                        String title,
                        String description,
                        String type) {
                return saveMqttNotification(title, description, type);
        }

        private NotificationResponse saveMqttNotification(
                        String title,
                        String description,
                        String type) {
                Notification saved = notificationRepository.save(Notification.builder()
                                .title(title)
                                .description(description)
                                .type(type)
                                .status("ACTIVE")
                                .build());
                NotificationResponse response = toResponse(saved);

                if (TransactionSynchronizationManager.isSynchronizationActive()) {
                        TransactionSynchronizationManager.registerSynchronization(
                                        new TransactionSynchronization() {
                                                @Override
                                                public void afterCommit() {
                                                        notificationRealtimeService.publish(response);
                                                }
                                        });
                } else {
                        notificationRealtimeService.publish(response);
                }

                return response;
        }

        @Override
        @Transactional(readOnly = true)
        public List<NotificationResponse> findAll() {

                return notificationRepository.findAll()
                                .stream()
                                .map(this::toResponse)
                                .toList();
        }

        @Override
        @Transactional(readOnly = true)
        public NotificationResponse findById(Long id) {
                return toResponse(getNotification(id));
        }

        @Override
        @Transactional(readOnly = true)
        public List<NotificationResponse> filter(
                        String status,
                        String type) {

                if (status != null && !status.isBlank()
                                && type != null && !type.isBlank()) {

                        return notificationRepository
                                        .findByStatusAndType(
                                                        status.trim(),
                                                        type.trim())
                                        .stream()
                                        .map(this::toResponse)
                                        .toList();
                }

                if (status != null && !status.isBlank()) {
                        return notificationRepository
                                        .findByStatus(status.trim())
                                        .stream()
                                        .map(this::toResponse)
                                        .toList();
                }

                if (type != null && !type.isBlank()) {
                        return notificationRepository
                                        .findByType(type.trim())
                                        .stream()
                                        .map(this::toResponse)
                                        .toList();
                }

                return findAll();
        }

        @Override
        @Transactional(readOnly = true)
        public List<NotificationResponse> searchByTitle(
                        String title) {

                return notificationRepository
                                .findByTitle(title.trim())
                                .stream()
                                .map(this::toResponse)
                                .toList();
        }

        @Override
        public NotificationResponse update(
                        Long id,
                        NotificationRequest request) {

                Notification notification = getNotification(id);

                if ("ARCHIVED".equalsIgnoreCase(
                                notification.getStatus())) {

                        throw new ResponseStatusException(
                                        HttpStatus.CONFLICT,
                                        "Une notification archivée ne peut pas être modifiée.");
                }

                notification.setTitle(request.title().trim());
                notification.setDescription(
                                request.description().trim());
                notification.setType(request.type().trim());

                return toResponse(
                                notificationRepository.save(notification));
        }

        @Override
        public NotificationResponse archive(Long id) {

                Notification notification = getNotification(id);

                notification.setStatus("ARCHIVED");

                return toResponse(
                                notificationRepository.save(notification));
        }

        @Override
        @Transactional
        public NotificationResponse updateReadStatus(Long id, boolean read) {
                Notification notification = getNotification(id);
                notification.setRead(read);

                return toResponse(notificationRepository.save(notification));
        }

        private Notification getNotification(Long id) {

                return notificationRepository.findById(id)
                                .orElseThrow(() -> new ResponseStatusException(
                                                HttpStatus.NOT_FOUND,
                                                "Notification introuvable : " + id));
        }

        private NotificationResponse toResponse(
                        Notification notification) {

                return new NotificationResponse(
                                notification.getId(),
                                notification.getTitle(),
                                notification.getDescription(),
                                notification.getTimestamp(),
                                notification.getType(),
                                notification.getStatus(),
                                notification.isRead());
        }

}
