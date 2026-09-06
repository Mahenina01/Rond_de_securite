#pragma once

#include <stdint.h>

/// Démarre le point d'accès SoftAP (protégé WPA2) et le serveur web
void softap_start();

/// Arrête le SoftAP, le serveur web, et invalide toute session active.
void softap_stop();

/**
 * @brief Définit le mot de passe applicatif utilisé pour authentifier l'accès à la page complète.
 * @param passwordPlain Mot de passe en clair, fourni une seule fois au
 *                       provisioning — ne jamais l'écrire en dur dans le code source.
 */
bool softap_set_password_hash(const char *passwordPlain);

void softap_loop();