package com.fortico.ronde_security.exception;

import jakarta.servlet.http.HttpServletRequest;
import jakarta.validation.ConstraintViolationException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.springframework.dao.DataIntegrityViolationException;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.http.converter.HttpMessageNotReadableException;
import org.springframework.validation.FieldError;
import org.springframework.web.bind.MethodArgumentNotValidException;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.RestControllerAdvice;
import org.springframework.web.method.annotation.MethodArgumentTypeMismatchException;

import java.util.LinkedHashMap;
import java.util.Map;

@RestControllerAdvice
public class GlobalExceptionHandler {

  private static final Logger log = LoggerFactory.getLogger(GlobalExceptionHandler.class);

  private ResponseEntity<ApiError> buildResponse(
      HttpStatus status,
      String message,
      HttpServletRequest request) {

    ApiError body = new ApiError(
        status.value(),
        status.getReasonPhrase(),
        message,
        request.getRequestURI());

    return ResponseEntity.status(status).body(body);
  }

  @ExceptionHandler(ResourceNotFoundException.class)
  public ResponseEntity<ApiError> handleNotFound(
      ResourceNotFoundException ex,
      HttpServletRequest request) {

    return buildResponse(
        HttpStatus.NOT_FOUND,
        ex.getMessage(),
        request);
  }

  @ExceptionHandler(MethodArgumentNotValidException.class)
  public ResponseEntity<ApiError> handleValidation(
      MethodArgumentNotValidException ex,
      HttpServletRequest request) {

    Map<String, String> errors = new LinkedHashMap<>();

    for (FieldError fieldError : ex.getBindingResult().getFieldErrors()) {
      errors.putIfAbsent(
          fieldError.getField(),
          fieldError.getDefaultMessage());
    }

    ApiError body = new ApiError(
        HttpStatus.BAD_REQUEST.value(),
        HttpStatus.BAD_REQUEST.getReasonPhrase(),
        "Les données envoyées sont invalides.",
        request.getRequestURI());

    body.setValidationErrors(errors);

    return ResponseEntity
        .badRequest()
        .body(body);
  }

  @ExceptionHandler(ConstraintViolationException.class)
  public ResponseEntity<ApiError> handleConstraintViolation(
      ConstraintViolationException ex,
      HttpServletRequest request) {

    return buildResponse(
        HttpStatus.BAD_REQUEST,
        "Les paramètres fournis ne respectent pas les contraintes.",
        request);
  }

  @ExceptionHandler(HttpMessageNotReadableException.class)
  public ResponseEntity<ApiError> handleUnreadableMessage(
      HttpMessageNotReadableException ex,
      HttpServletRequest request) {

    return buildResponse(
        HttpStatus.BAD_REQUEST,
        "Le corps de la requête est absent ou son format JSON est invalide.",
        request);
  }

  @ExceptionHandler(MethodArgumentTypeMismatchException.class)
  public ResponseEntity<ApiError> handleTypeMismatch(
      MethodArgumentTypeMismatchException ex,
      HttpServletRequest request) {

    return buildResponse(
        HttpStatus.BAD_REQUEST,
        "Le paramètre '" + ex.getName()
            + "' possède un type invalide.",
        request);
  }

  @ExceptionHandler(DataIntegrityViolationException.class)
  public ResponseEntity<ApiError> handleDataIntegrity(
      DataIntegrityViolationException ex,
      HttpServletRequest request) {

    log.warn("Violation d'intégrité en base de données", ex);

    return buildResponse(
        HttpStatus.CONFLICT,
        "L'opération ne peut pas être effectuée en raison d'un conflit de données.",
        request);
  }

  @ExceptionHandler(Exception.class)
  public ResponseEntity<ApiError> handleUnexpectedException(
      Exception ex,
      HttpServletRequest request) {

    log.error(
        "Erreur interne sur {}",
        request.getRequestURI(),
        ex);

    return buildResponse(
        HttpStatus.INTERNAL_SERVER_ERROR,
        "Une erreur interne est survenue.",
        request);
  }
}
