package com.fortico.ronde_security.repo;

import com.fortico.ronde_security.model.Location;
import org.springframework.data.jpa.repository.JpaRepository;
import java.util.Optional;
import java.util.List;

public interface LocationRepository extends JpaRepository<Location, Long> {

    Optional<Location> findByCheckpointCode(String checkpointCode);

    boolean existsByCheckpointCode(
            String checkpointCode);

    boolean existsByCheckpointCodeAndIdNot(
            String checkpointCode,
            Long id);

    List<Location> findByAgentId(Long agentId);

    List<Location> findByNameContainingIgnoreCase(String agentName);

    List<Location> findByAgent_Id(Long agentId);

    List<Location> findByAgentIsNull();

}
