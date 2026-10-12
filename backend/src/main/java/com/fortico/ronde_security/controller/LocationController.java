package com.fortico.ronde_security.controller;

import lombok.RequiredArgsConstructor;

import com.fortico.ronde_security.dtos.location.*;
import com.fortico.ronde_security.service.location.LocationService;

import org.springframework.http.HttpStatus;
import org.springframework.validation.annotation.Validated;

import jakarta.validation.Valid;
import jakarta.validation.constraints.Positive;
import java.util.List;
import org.springframework.web.bind.annotation.*;

@RestController 
@RequestMapping("/api/v1/locations")
@RequiredArgsConstructor 
@Validated 
public class LocationController {
    private final LocationService locationService;

    @PostMapping
    @ResponseStatus(HttpStatus.CREATED)
    public LocationResponse create(
        @Valid @RequestBody LocationRequest request
    ){
        return locationService.create(request);
    }

    @GetMapping
    public List<LocationResponse> getAll(
        @RequestParam(required = false) String search
    ){
        if(search != null && !search.isBlank()){
            return locationService.search(search);
        }
        return locationService.getAll();
    }

    @GetMapping("/{id}")
    public LocationResponse findById(
        @PathVariable @Positive Long id
    ){
        return locationService.findById(id);
    }

    @PutMapping("/{id}")
    public LocationResponse update(
        @PathVariable @Positive Long id,
        @Valid @RequestBody LocationRequest request
    ){
        return locationService.update(id, request);
    }

    @PatchMapping ("/{id}/status") 
    public LocationResponse updateStatus(
        @PathVariable @Positive Long id,
        @RequestParam boolean isActive
    ){
        return locationService.changeStatus(id, isActive);
    }

    @PatchMapping("/{id}/agent")
    public LocationResponse assignAgent(
            @PathVariable @Positive Long id,
            @RequestParam @Positive Long agentId) {

        return locationService.assignAgent(id, agentId);
    }

    @GetMapping("/by-agent/{agentId}")
    public List<LocationResponse> findByAgent(
            @PathVariable @Positive Long agentId) {

        return locationService.findByAgent(agentId);
    }

    
    @DeleteMapping("/{id}")
    @ResponseStatus(HttpStatus.NO_CONTENT)
    public void delete(
            @PathVariable @Positive Long id) {

        locationService.delete(id);
    }



}
