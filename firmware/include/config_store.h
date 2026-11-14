#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H

#include <stdbool.h>
#include <stddef.h>

// Réglages persistants du boîtier, stockés dans la NVS (Preferences) et non
// dans LittleFS : ils survivent à un "uploadfs" et à l'effacement des logs.

#define PASSWORD_MIN_LEN 8
#define PASSWORD_MAX_LEN 64
#define DEVICE_ID_MAX_LEN 24

// À appeler une fois au démarrage.
void config_init(void);

// Mot de passe du tableau de bord. Tant qu'aucun mot de passe n'a été
// choisi, c'est DASHBOARD_DEFAULT_PASSWORD (par défaut RESCUE_PASS).
// Le mot de passe est stocké haché (SHA-256 + sel aléatoire), jamais en clair.
bool config_check_password(const char *plain);
bool config_set_password(const char *plain); // 8 à 64 caractères
bool config_password_is_default(void);

// Synchronisation automatique de l'heure par le réseau mobile (par défaut : oui).
bool config_auto_time_sync(void);
void config_set_auto_time_sync(bool enabled);

// Identifiant du boîtier utilisé par les topics et le client MQTT.
// Caractères autorisés : A-Z, 0-9, tiret et souligné.
const char *config_device_id(void);
bool config_device_id_valid(const char *device_id);
bool config_set_device_id(const char *device_id);

#endif