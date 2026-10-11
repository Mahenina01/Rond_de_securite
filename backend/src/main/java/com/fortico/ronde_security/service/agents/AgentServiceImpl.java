package com.fortico.ronde_security.service.agents;

import com.fortico.ronde_security.dtos.agent.AgentRequest;
import com.fortico.ronde_security.dtos.agent.AgentResponse;
import com.fortico.ronde_security.exception.DuplicateAgentCodeException;
import com.fortico.ronde_security.exception.ResourceNotFoundException;
import com.fortico.ronde_security.repo.AgentRepository;
import com.fortico.ronde_security.model.Agents;
import java.util.List;
import java.util.stream.Collectors;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

@Service
public class AgentServiceImpl implements AgentService {

    private final AgentRepository agentRepository;

    public AgentServiceImpl(AgentRepository agentRepository) {
        this.agentRepository = agentRepository;
    }

    @Override
    public AgentResponse create(AgentRequest request) {
        if (agentRepository.existsByAgentCode(request.agentCode())) {
            throw new DuplicateAgentCodeException("Le code de l'agent existe déjà : " + request.agentCode());
        }
        Agents agent = new Agents();
        agent.setAgentCode(request.agentCode());
        agent.setName(request.name());
        agent.setPhone(request.phone());
        agent.setIsActive(true);

        Agents savedAgent = agentRepository.save(agent);

        return toAgentResponse(savedAgent);
    }

    @Override
    @Transactional(readOnly = true)
    public List<AgentResponse> getAll() {
        return agentRepository.findAll()
                .stream()
                .map(this::toAgentResponse)
                .toList();
    }

    @Override
    @Transactional(readOnly = true)
    public AgentResponse findById(Long id) {
        Agents agent = getAgentById(id);
        return toAgentResponse(agent);
    }

    private Agents getAgentById(Long id) {
        return agentRepository.findById(id)
                .orElseThrow(() -> new ResourceNotFoundException("Agent not found with id: " + id));
    }

    @Override
    public AgentResponse update(Long id, AgentRequest request) {
        Agents agent = getAgentById(id);
        if (agentRepository.existsByAgentCodeAndIdNot(request.agentCode(), id)) {
            throw new DuplicateAgentCodeException("Le code de l'agent existe déjà : " + request.agentCode());
        }
        agent.setAgentCode(request.agentCode());
        agent.setName(request.name());
        agent.setPhone(request.phone());
        Agents updatedAgent = agentRepository.save(agent);
        return toAgentResponse(updatedAgent);
    }

    @Override
    public AgentResponse updateStatus(Long id, Boolean isActive) {
        Agents agent = getAgentById(id);
        agent.setIsActive(isActive);
        Agents updatedAgent = agentRepository.save(agent);
        return toAgentResponse(updatedAgent);
    }

    @Override
    public void delete(Long id) {
        Agents agent = getAgentById(id);
        agentRepository.delete(agent);
    }

    @Override
    public List<AgentResponse> searchByName(String name) {
        List<Agents> agents = agentRepository.findByNameContainingIgnoreCase(name);
        return agents.stream()
                .map(this::toAgentResponse)
                .collect(Collectors.toList());
    }

    private AgentResponse toAgentResponse(Agents agent) {
        return new AgentResponse(
                agent.getId(),
                agent.getAgentCode(),
                agent.getName(),
                agent.getPhone(),
                agent.getIsActive());
    }

}
