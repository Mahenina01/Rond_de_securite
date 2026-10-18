package com.fortico.ronde_security.service.location;

import com.fortico.ronde_security.dtos.location.LocationRequest;
import com.fortico.ronde_security.dtos.location.LocationResponse;
import java.util.List;

public interface LocationService {
    LocationResponse create(LocationRequest request);

    List<LocationResponse> getAll();

    LocationResponse findById(Long id);

    LocationResponse update(Long id, LocationRequest request);

    LocationResponse changeStatus(
            Long id,
            boolean active);

    LocationResponse assignAgent(
            Long locationId,
            Long agentId);

    List<LocationResponse> findByAgent(Long agentId);

    List<LocationResponse> search(String name);

    void delete(Long id);

}
