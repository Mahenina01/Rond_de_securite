
package com.fortico.ronde_security.repo;

import java.util.List;
import java.util.Optional;

import org.springframework.data.jpa.repository.JpaRepository;

import com.fortico.ronde_security.model.Devices;

public interface DevicesRepository extends JpaRepository<Devices, Long> {

  // Vérifier si un code de dispositif existe
  boolean existsByDeviceCode(String deviceCode);

  // Vérifier si un code de dispositif existe pour un autre dispositif
  boolean existsByDeviceCodeAndIdNot(String deviceCode, Long id);

  // Rechercher un dispositif par son code
  Optional<Devices> findByDeviceCode(String deviceCode);

  // Rechercher tous les dispositifs associés à un agent spécifique
  List<Devices> findByAgent_AgentCode(String agentCode);

}
