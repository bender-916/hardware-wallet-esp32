# REFACTOR ARQUITECTURA - Hardware Wallet ESP32-S3

## 1. MEDIDAS DE SEGURIDAD ESP32-S3 NATIVO

### 1.1 Flash Encryption XTS-AES-256
- Algoritmo XTS-AES-128/256 para cifrado de flash externa
- Claves derivadas de HMAC-based derivation (HMAC_KEY0-5)
- Cifrado transparente con 4K sector size
- Previene lectura física de la flash

### 1.2 Secure Boot V2 ECDSA P-256
- Bootloader firmado con ECDSA usando curva P-256
- Clave pública quemada en efuse (BLOCK2)
- Verificación de cadena de confianza: BootROM -> Bootloader -> App
- Previene ejecución de código no firmado

### 1.3 HMAC Submodule efuse Keys
- HMAC_KEY0-5 almacenadas en efuse (una sola escritura)
- Usado para derivación de claves de cifrado
- Accesible solo por hardware, no por software

### 1.4 Digital Signature Peripheral
- Acelerador de firma RSA/ECC
- Claves privadas nunca salen del HSM interno
- Soporte ECDSA P-256 para firma de transacciones

### 1.5 JTAG Disable efuse
- Fuse DIS_USB_JTAG para deshabilitar JTAG sobre USB
- Fuse DIS_PAD_JTAG para deshabilitar JTAG físico
- Previene debugging y extracción de memoria

### 1.6 Download Mode Disable
- Fuse SECURE_DOWNLOAD_MODE para restringir flash chip
- Modo UART limitado solo para carga de apps firmadas
- Previene lectura/escritura directa de flash

### 1.7 DRAM Encryption
- Datos en RAM cifrados automáticamente
- Protección contra cold boot attacks
- Claves internas del chip, no accesibles

### 1.8 Anti-rollback
- Versión de seguridad (SECURE_VERSION) en efuse
- Rechaza actualizaciones a versiones anteriores
- Previene downgrade attacks

---

## 2. ARQUITECTURA DE MEMORIA SEGURA

### 2.1 Mnemonic: derivada al vuelo, cacheada mínimo tiempo
- La frase semilla (BIP-39) se deriva de entropy sólo cuando es necesaria
- Tiempo de vida: únicamente durante operación criptográfica activa
- Post-operación: buffer sobreescrito con zeros (memset_s o similar)
- Nunca se serializa a flash ni a buffer persistente

### 2.2 Private keys: solo en RAM, cifrada por DRAM encryption
- Claves privadas BIP-32 generadas dinámicamente desde mnemonic
- Existencia temporal: durante firma de transacción únicamente
- Almacenamiento: stack heap RAM (no en variables globales estáticas)
- DRAM encryption proporciona capa adicional de protección
- Zeroización inmediata tras uso con explicit_memzero

### 2.3 PIN: zero después de verificación
- Buffer de entrada de PIN limpio tras cada dígito
- Valor completo de PIN nunca almacenado en variable
- Comparación hash (o directa) con constant-time operations
- Post-verificación: todos los buffers con PIN se limpian
- Rate limiting hardware/software previene brute force

### 2.4 Seed phrase: nunca en flash plano
- NUNCA almacenar el mnemonico en flash sin cifrar
- NUNCA hacer logs del mnemonico
- Recuperación solo vía re-entrada de usuario
- Backup físico: palabras mostradas en pantalla durante setup inicial
- No existe "recuperación desde backup" - el usuario debe re-introducir

---

## 3. MÁQUINA DE ESTADOS

```
                    +-------+
                    |  BOOT |
                    +---+---+
                        |
                        v
+---------+      +-------------+      +----------+      +-----------+
|  LOCKED |<-----| PIN_ENTRY   |----->| UNLOCKED |----->|  SIGNING  |
| (apagado|      | (rate limit)|      | (wallet  |      |(confirm   |
|pantalla)|      +-------------+      |  ready)  |      | required) |
+---------+      ^        |           +----+-----+      +-----+-----+
     ^           |        |                |                |
     |           |        +----------------+                |
     |           |        (pin correcto)    |                 |
     +-----------+                        (sign tx)           |
     (timeout/                              |               (cancel/
      lock cmd)                             v                confirm)
                                      +-------------+         |
                                      |  SIGNING    |<--------+
                                      |  CONFIRMED  |
                                      +-------------+
                                           |
                                           v
                                      +-------------+
                                      |  RETURN TO  |
                                      |  UNLOCKED   |
                                      +-------------+
```

### 3.1 BOOT
- Inicialización del sistema
- Verificación de integridad (hash de firmware)
- Transición automática a LOCKED

### 3.2 LOCKED (pantalla apagada)
- Estado por defecto tras inicio o timeout
- Pantalla apagada para ahorro y privacidad
- Solo acepta: botón de despertar
- Transición a: PIN_ENTRY

### 3.3 PIN_ENTRY (con rate limiting)
- Espera entrada de PIN de usuario
- Rate limiting: delay exponencial entre intentos (100ms, 1s, 5s, 30s, 60s)
- Máximo 10 intentos antes de wipe completo (configurable)
- Transición exitosa: UNLOCKED
- Transición fallida: LOCKED (después de delay)

### 3.4 UNLOCKED (wallet disponible)
- Wallet operativo
- Visualización de direcciones, balance
- Preparación de transacciones (sin firmar)
- Transición a: SIGNING (cuando se solicita firmar)
- Timeout a: LOCKED (configurable, default 2 min)

### 3.5 SIGNING (confirmación física requerida)
- Transacción preparada mostrada en pantalla
- Requisito: confirmación física (botón ACEPTAR)
- Opción: cancelar (botón CANCELAR)
- Mientras tanto: private key derivada en RAM
- Post-confirmación: firma, zeroización, retorno a UNLOCKED

---

## 4. PROTECCIONES ANTI-ATAQUE

### 4.1 Constant-time operations
- Comparación de PIN: CRYPTO_memcmp (no memcmp)
- Operaciones criptográficas: librerías constant-time
- Evita side-channel timing attacks
- Evita power analysis por tiempo

### 4.2 JTAG disable
- Efuses quemadas en producción:
  - DIS_PAD_JTAG = 1 (JTAG físico deshabilitado)
  - DIS_USB_JTAG = 1 (JTAG sobre USB deshabilitado)
- Previene debugging en producción
- Previene lectura de RAM en vivo

### 4.3 Power glitch delays
- Delay aleatorio antes de operaciones críticas
- Verificación de integridad del firmware
- Watchdog con reset seguro
- Glitch detectors internos del ESP32-S3

### 4.4 Cold boot zero RAM
- Al inicio (bootloader custom): memset de toda la RAM
- Limpieza de potenciales restos de claves anteriores
- Fortalecido por DRAM encryption del chip

### 4.5 Tamper detection GPIO
- GPIOs configurados como sensores de manipulación
- Switch físico en carcasa (apertura detectable)
- Reacción al tampering: wipe de claves en RAM, pantalla de emergencia
- Opcional: sensor de temperatura/luz para ataques láser

---

## 5. ESTRUCTURA DE DIRECTORIOS REFACTOR

```
firmware/
├── CMakeLists.txt
├── sdkconfig.defaults
└── main/
    ├── CMakeLists.txt
    ├── main.c                  # Boot seguro, entry point
    ├── security/
    │   ├── flash_enc.c         # Configuración Flash Encryption
    │   ├── flash_enc.h
    │   ├── secure_boot.c       # Configuración Secure Boot V2
    │   ├── secure_boot.h
    │   ├── hmac.c              # Operaciones HMAC con efuse keys
    │   ├── hmac.h
    │   ├── tamper.c            # Detección de manipulación
    │   └── tamper.h
    ├── wallet/
    │   ├── state_machine.c     # Máquina de estados principal
    │   ├── state_machine.h
    │   ├── key_management.c    # Derivación BIP-32, gestión claves
    │   ├── key_management.h
    │   ├── pin.c               # Verificación PIN segura
    │   ├── pin.h
    │   └── wipe.c              # Secure wipe / factory reset
    ├── crypto/
    │   ├── bip39.c             # Mnemonic generation/validation
    │   ├── bip39.h
    │   ├── bip32.c             # HD wallet derivation (constant-time)
    │   ├── bip32.h
    │   ├── psbt.c              # Parsing y firma PSBT
    │   ├── psbt