package com.fortico.ronde_security.controller;

import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseStatus;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.http.HttpStatus;
import org.springframework.validation.annotation.Validated;
import lombok.RequiredArgsConstructor;

import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import jakarta.validation.Valid;
import jakarta.validation.constraints.Positive;

import com.fortico.ronde_security.dtos.agent.AgentRequest;
import com.fortico.ronde_security.dtos.agent.AgentResponse;
import com.fortico.ronde_security.service.agents.AgentService;

import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PatchMapping;
import org.springframework.web.bind.annotation.PathVariable;

import java.util.List;

@RestController
@RequestMapping("/api/v1/agents")
@RequiredArgsConstructor
@Validated
public class AgentController {
    private final AgentService agentService;

    @PostMapping
    @ResponseStatus(org.springframework.http.HttpStatus.CREATED)
    public AgentResponse create(
            @Valid @RequestBody AgentRequest request) {
        return agentService.create(request);
    }

    @GetMapping
    public List<AgentResponse> getAll(
            @RequestParam(required = false) String search) {
        if (search != null && !search.isBlank()) {
            return agentService.searchByName(search);
        }
        return agentService.getAll();
    }

    @GetMapping("/{id}")
    public AgentResponse findById(
            @PathVariable @Positive Long id) {

        return agentService.findById(id);
    }

    @PatchMapping("/{id}")
    public AgentResponse update(
            @PathVariable @Positive Long id,
            @Valid @RequestBody AgentRequest request) {

        return agentService.update(id, request);
    }

    @PatchMapping("/{id}/status")
    public AgentResponse updateStatus(
            @PathVariable @Positive Long id,
            @RequestParam boolean active) {

        return agentService.updateStatus(id, active);
    }

    @DeleteMapping("/{id}")
    @ResponseStatus(HttpStatus.NO_CONTENT)
    public void delete(
            @PathVariable @Positive Long id) {

        agentService.delete(id);
    }

}
