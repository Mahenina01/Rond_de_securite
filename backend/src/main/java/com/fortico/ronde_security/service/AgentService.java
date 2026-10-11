package com.fortico.ronde_security.service;

import java.util.List;

import com.fortico.ronde_security.dtos.Agent.AgentRequest;
import com.fortico.ronde_security.dtos.Agent.AgentResponse;
public interface AgentService {
    AgentResponse create(AgentRequest request);

    List<AgentResponse> getAll();

    AgentResponse findById(Long id);

    AgentResponse update(Long id, AgentRequest request);

    AgentResponse updateStatus(Long id, Boolean isActive);

    void delete(Long id);

    List<AgentResponse> searchByName(String name);
}
