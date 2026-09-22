#pragma once

void softap_start();
void softap_loop();
void softap_stop();
bool softap_set_password_hash(const char *passwordPlain);