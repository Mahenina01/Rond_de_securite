package com.fortico.ronde_security.repo;

import java.util.List;
import java.util.Optional;

import org.springframework.data.jpa.repository.JpaRepository;

import com.fortico.ronde_security.model.Agents;

public interface AgentRepository extends JpaRepository<Agents, Long> {
    Optional<Agents> findByAgentCode(String agentCode);

    Optional<Agents> findByName(String name);

    boolean existsByAgentCode(String agentCode);

    boolean existsByAgentCodeAndIdNot(String agentCode, Long id);

    List<Agents> findByIsActive(boolean isActive);

    List<Agents> findByNameContainingIgnoreCase(String name);

    Optional<Agents> findByNameIgnoreCase(String name);
}
