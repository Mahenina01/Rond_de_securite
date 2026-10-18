package com.fortico.ronde_security.service.location;

import com.fortico.ronde_security.dtos.location.LocationRequest;
import com.fortico.ronde_security.dtos.location.LocationResponse;
import com.fortico.ronde_security.dtos.agent.AgentResponse;

import com.fortico.ronde_security.exception.ResourceNotFoundException;
import com.fortico.ronde_security.exception.DuplicateAgentCodeException;

import com.fortico.ronde_security.repo.AgentRepository;
import com.fortico.ronde_security.repo.LocationRepository;

import com.fortico.ronde_security.model.Agents;
import com.fortico.ronde_security.model.Location;

import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.util.List;

@Service
@RequiredArgsConstructor
@Transactional
public class LocationServiceImpl implements LocationService {

    private final LocationRepository locationRepository;
    private final AgentRepository agentRepository;

    @Override
    public LocationResponse create(LocationRequest request) {

        String code = request.checkpointCode().trim();

        if (locationRepository.existsByCheckpointCode(code)) {
            throw new DuplicateAgentCodeException(
                    "Ce code de point de contrôle existe déjà : " + code);
        }

        Agents agent = findAgentByName(request.agentName());

        Location location = Location.builder()
                .checkpointCode(request.checkpointCode().trim())
                .name(request.name().trim())
                .agent(agent)
                .isActive(true)
                .build();

        Location saved = locationRepository.save(location);

        return toResponse(saved);
    }

    @Override
    @Transactional(readOnly = true)
    public List<LocationResponse> getAll() {

        return locationRepository.findAll()
                .stream()
                .map(this::toResponse)
                .toList();
    }

    @Override
    @Transactional(readOnly = true)
    public LocationResponse findById(Long id) {

        return toResponse(findLocation(id));
    }

    @Override
    public LocationResponse update(
            Long id,
            LocationRequest request) {

        Location location = findLocation(id);

        String code = request.checkpointCode().trim();

        if (locationRepository.existsByCheckpointCodeAndIdNot(
                code, id)) {

            throw new DuplicateAgentCodeException(
                    "Ce code est déjà utilisé : " + code);
        }

        Agents agent = findAgentByName(request.agentName());

        location.setCheckpointCode(request.checkpointCode().trim());
        location.setName(request.name().trim());
        location.setAgent(agent);

        Location updated = locationRepository.save(location);

        return toResponse(updated);
    }

    @Override
    public LocationResponse changeStatus(
            Long id,
            boolean active) {

        Location location = findLocation(id);

        location.setIsActive(active);

        return toResponse(locationRepository.save(location));
    }

    @Override
    public LocationResponse assignAgent(
            Long locationId,
            Long agentId) {

        Location location = findLocation(locationId);
        Agents agent = findAgent(agentId);

        location.setAgent(agent);

        Location updated = locationRepository.save(location);

        return toResponse(updated);
    }

    @Override
    @Transactional(readOnly = true)
    public List<LocationResponse> findByAgent(Long agentId) {

        findAgent(agentId);

        return locationRepository.findByAgent_Id(agentId)
                .stream()
                .map(this::toResponse)
                .toList();
    }

    @Override
    @Transactional(readOnly = true)
    public List<LocationResponse> search(String name) {

        return locationRepository
                .findByNameContainingIgnoreCase(name.trim())
                .stream()
                .map(this::toResponse)
                .toList();
    }

    @Override
    public void delete(Long id) {

        Location location = findLocation(id);

        /*
         * Avant une suppression physique, vérifier
         * les références dans Logs et Alerts.
         *
         * Une désactivation est préférable lorsque
         * l'emplacement possède un historique.
         */

        locationRepository.delete(location);
    }

    private Location findLocation(Long id) {

        return locationRepository.findById(id)
                .orElseThrow(() -> new ResourceNotFoundException(
                        "Emplacement introuvable : " + id));
    }

    private Agents findAgent(Long agentId) {

        return agentRepository.findById(agentId)
                .orElseThrow(() -> new ResourceNotFoundException(
                        "Agent introuvable : " + agentId));
    }

    private LocationResponse toResponse(Location location) {

        AgentResponse agentResponse = null;

        if (location.getAgent() != null) {
            Agents agent = location.getAgent();

            agentResponse = new AgentResponse(
                    // À adapter selon les champs du record AgentResponse
                    agent.getId(),
                    agent.getAgentCode(),
                    agent.getName(),
                    agent.getPhone(),
                    agent.getIsActive());
        }

        return new LocationResponse(
                location.getId(),
                location.getCheckpointCode(),
                location.getName(),
                agentResponse,
                location.getIsActive());
    }

    private Agents findAgentByName(String name) {
        return agentRepository.findByNameIgnoreCase(name.trim())
                .orElseThrow(() -> new ResourceNotFoundException(
                        "Agent introuvable : " + name));
    }
}