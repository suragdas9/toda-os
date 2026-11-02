/**
 * Toda OS — Security Layer
 * Boot Chain Verification
 *
 * Verifies that the OS has NOT been tampered with.
 * If anything is modified → REFUSE to boot.
 * This is what makes Toda OS fully locked.
 *
 * Copyright (c) Toda OS Project
 */

#ifndef TODA_SECURITY_H
#define TODA_SECURITY_H

#include "../../core/kernel.h"

/* Verification result codes */
#define VERIFY_OK           0
#define VERIFY_FAIL         -1
#define VERIFY_TAMPERED     -2

/* Boot chain verification */
result_t boot_verify_chain(void);
result_t boot_verify_signature(void* data, size_t len, uint8_t* sig);

/* Secure storage */
result_t secure_key_store(uint8_t* key, size_t len);
result_t secure_key_read(uint8_t* buf, size_t len);

/* Runtime integrity */
void security_init(void);
bool security_check_process(uint32_t pid);

#endif /* TODA_SECURITY_H */

