# Análisis de Seguridad - Hardware Wallet ESP32

> **Contexto:** ESP32-S3 con Flash Encryption + Secure Boot V2 (sin HSM dedicado)

---

## 1. Limitaciones del ESP32-S3

### 1.1 Flash Encryption

| Aspecto | Detalle |
|---------|---------|
| **Qué hace** | Cifra el contenido de la flash externa (XTS-AES-128 o XTS-AES-256) |
| **Generación de clave** | Clave cifrada por hardware en fábrica (efuse) + clave de cifrado del usuario |
| **Protección** | Impide lectura de firmware/binarios desde la flash física |
| **Limitaciones** | No protege runtime (RAM); ataques de debug; glitching |

**Proceso:**
```
Bootloader → Descifra flash → Carga en RAM → Ejecuta
```

**Vulnerabilidades conocidas:**
- La clave de cifrado de flash se almacena en efuses (no extraíble pero accesible al SoC)
- Ataques de power glitching pueden saltar verificaciones
- Debug JTAG cerrado pero potencialmente reactivable
- No hay protección contra lectura de RAM en runtime

### 1.2 Secure Boot V2

| Aspecto | Detalle |
|---------|---------|
| **Algoritmo** | ECDSA con curva P-256 |
| **Validación** | Firma RSA-3072 o ECDSA P-256 en bootloader y app |
| **Protección** | Prevents unauthorized firmware execution |
| **Organización** | Chain of trust: BootROM → Bootloader → App |

**Proceso de secure boot:**
```
BootROM (ROM, inmutable)
    ↓
Verifica firma del Bootloader con key en efuse
    ↓
Bootloader verifica firma de la aplicación
    ↓
Ejecuta aplicación
```

**Vulnerabilidades:**
- Key revocation limitada
- No recovery ante compromise de clave de firma
- Ataques de electromagnetismo/powe analysis en verificación

### 1.3 Qué NO Protege el ESP32-S3

| Tipo de Ataque | Riesgo | Mitigación |
|----------------|--------|------------|
| **Side-channel (PA/DPA)** | Alto - Consumo analizable para extraer claves | Secure Element externo |
| **Glitching (power/clock)** | Alto - Bypass de verificaciones | Hardware externo + tamper detection |
| **Physical extraction** | Medio - Decapacitado + lectura de efuses difícil pero posible | Epoxy, casing |
| **JTAG re-enable** | Medio - Si el atacante puede escribir efuses | Bloqueo permanente eFuses |
| **RAM dumping** | Medio - Volcado de RAM en runtime | Secure Element con claves nunca en RAM |
| **EM analysis** | Medio - Lectura EM del procesamiento de claves | Shielding, SE externo |

**Conclusión:** El ESP32-S3 proporciona **seguridad de software** (cifrado + firmado) pero no **seguridad de hardware** (HSM) para proteger claves privadas.

---

## 2. Opciones de Secure Element Externo

### 2.1 Comparativa de Secure Elements

| Característica | ATECC608A | ATECC608B | SE050 | STSAFE-A110 |
|----------------|-----------|-----------|-------|-------------|
| **Fabricante** | Microchip | Microchip | NXP | STMicro |
| **Precio unitario** | $2-4 | $3-5 | $5-8 | $3-6 |
| **Bus de conexión** | I2C (1MHz) | I2C (1MHz) | I2C/SPI | I2C |
| **Tamper detection** | Sí | Sí | Sí | Sí |
| **ECDSA P-256** | ✅ | ✅ | ✅ | ✅ |
| **Compresión de clave** | ✅ | ✅ | ✅ | ✅ |
| **Slots de clave** | 16 configurables | 16 configurables | Extensa | 3+ zonas |
| **Random number** | ✅ DRBG | ✅ DRBG | ✅ NIST | ✅ TRNG |

### 2.2 ATECC608A/B (Microchip)

**Conexión ESP32:**
```
ESP32-S3          ATECC608
--------          --------
GPIO (SDA)  ----> SDA (I2C)
GPIO (SCL)  ----> SCL (I2C)
3.3V        ----> VCC
GND         ----> GND
```

**Operaciones soportadas:**
- Generación de claves ECDSA P-256 (no exportable)
- Firma ECDSA (la clave privada nunca sale del chip)
- Verificación de firma
- MAC/HMAC
- Claves derivadas (ECDH)
- Random number generation
- AES-128/GCM (608B)

**Librerías disponibles:**
- [cryptoauthlib](https://github.com/MicrochipTech/cryptoauthlib) - Oficial de Microchip (C)
- [ArduinoECCX08](https://github.com/arduino-libraries/ArduinoECCX08) - Arduino wrapper
- [esp_cryptoauth_lib](https://github.com/espressif/esp-cryptoauthlib) - Port para ESP-IDF

**Ventajas:**
- Muy probado, usado por Arduino MKR
- Barato y amplio soporte
- Librería bien documentada
- Secure boot con ESP32 integrado

**Datasheet:** [ATECC608A](https://ww1.microchip.com/downloads/en/DeviceDoc/ATECC608A-CryptoAuthentication-Device-Summary-DataSheet-DS40001977B.pdf)

### 2.3 SE050 (NXP)

**Conexión ESP32:**
```
ESP32-S3          SE050
--------          -----
GPIO (SDA)  ----> SDA (I2C at 3.4MHz)
GPIO (SCL)  ----> SCL
GPIO (CS)   ----> CS (SPI option)
GPIO (MOSI) ----> MOSI
GPIO (MISO) ----> MISO
GPIO (SCK)  ----> SCK
3.3V        ----> VDD
GND         ----> GND
```

**Operaciones soportadas:**
- ECDSA P-256/P-384/P-521
- Ed25519
- RSA hasta 4096
- AES-256/GCM
- SHA-384/SHA-512
- HKDF, PBKDF2
- TLS 1.3 handshake completo
- Mayor memoria para certs/applets

**Librerías disponibles:**
- [se050](https://github.com/NXP/plug-and-trust) - Plug & Trust MW (C)
- Middleware específico NXP

**Ventajas:**
- Más algoritmos modernos (Ed25519)
- Más memoria (ideal para múltiples apps)
- Pre-certificado (Common Criteria EAL6+)
- Soporte I2C a 3.4MHz (Fast Mode+)

**Datasheet:** [SE050](https://www.nxp.com/products/security-and-authentication/authenticated-security-controllers/secure-element-solutions/secure-element-with-nfc-interface-and-esim-support:SE050)

### 2.4 STSAFE-A110 (STMicro)

**Conexión ESP32:**
```
ESP32-S3        STSAFE-A110
--------        -----------
GPIO (SDA) ---> SDA
GPIO (SCL) ---> SCL
3.3V       ---> VCC
GND        ---> GND
```

**Operaciones soportadas:**
- ECDSA P-256 (sign + verify)
- ECDH shared secret
- AES-128/CCM
- HMAC-SHA256
- Secure counting
- Personalization segura

**Librerías disponibles:**
- [STSW-STSA110](https://www.st.com/en/secure-mcus/stsafe-a110.html) - Oficial ST (C)
- Ejemplos para STM32 (portable a ESP32)

**Ventajas:**
- Certificación Common Criteria EAL5+
- Optimizado para IoT
- Low power consumption
- Soporte de autenticación mutua

**Datasheet:** [STSAFE-A110](https://www.st.com/resource/en/datasheet/stsafe-a110.pdf)

---

## 3. Medidas de Seguridad Física

### 3.1 Tamper Detection

| Sensor | Implementación | Coste |
|--------|----------------|-------|
| **Microswitch** | Contacto si se abre carcasa | $0.50-1 |
| **Light sensor** | Fotodiodo detecta luz | $0.30-0.50 |
| **Mesh conductor** | Malla impresa en PCB | $0.20-0.50 |
| **Temp sensor** | Detecta cambio brusco | Integrado |
| **Accelerometer** | Detección de movimiento | $1-2 |

**Circuito ejemplo:**
```
+3.3V ----[Switch]----[10k]--- GPIO
                      |
                     GND
```
Cuando el switch se abre (carcasa), GPIO al HIGH → Trigger wipe.

### 3.2 Epoxy / Tamper-Evident Casing

| Tipo | Descripción | Coste |
|------|-------------|-------|
| **Epoxy negro** | Relleno del case, difícil remover sin dañar PCB | $1-3 |
| **Cinta destructiva** | Cinta evidenciadora en tornillos | $0.50-1 |
| **Case sellado** | Ultrasónicamente sellado | $3-5 |
| **Etiqueta holográfica** | Sellos de seguridad | $0.20-0.50 |

**Recomendación:** Epoxy conductivo con partículas metálicas combinado con mesh tamper.

### 3.3 Shielding EM

| Método | Eficacia | Coste |
|--------|----------|-------|
| **Cage de Faraday** | Alta | $2-5 |
| **Spray conductor** | Media | $0.50-1 |
| **Foil de cobre** | Media-Alta | $0.30-0.50 |
| **PCB de 4 capas** con planos | Media | Extra en PCB |

**Implementación:** Case metálico conectorizado a GND + gaskets conductivos.

### 3.4 Borrado Seguro por USB (Secure Wipe)

**Mecanismo:**
```
USB Host → Comando especial → Secure Erase SE
                              → Overwrite flash
                              → Clear RAM
                              → Reset efuse flags (si posible)
```

**Combinación con tamper:**
```c
// Pseudocode
void tamper_isr() {
    secure_element_wipe_keys();
    flash_erase_all();  // Or secure partition
    esp_restart();
}
```

**Coste:** Circuito adicional ~$0.50

---

## 4. Comparativa con Ledger/Trezor

### 4.1 Ledger (Nano S, Nano X, Stax)

| Componente | Especificación |
|------------|----------------|
| **Secure Element** | ST33J2M0 (STMicro) |
| **Certificación** | Common Criteria EAL6+ |
| **Procesador aplicación** | STM32 (propio) |
| **Conectividad** | USB, Bluetooth (Nano X), NFC (Stax) |
| **Pantalla** | OLED 128x64 / 128x128 |

**Características ST33:**
- Secure Element de grado bancario
- Ejecución de código en zona segura (TrustZone)
- Anti-tampering físico integrado
- Memoria cifrada persistente
- Generación de claves protegida
- **NO disponible para compra retail** - Solo bajo NDA y volumen

**Coste estimado del chip:** $15-25 (estimado, no precio público)

### 4.2 Trezor Model T / One

| Componente | Especificación |
|------------|----------------|
| **Secure Element** | **NINGUNO** (diseño "open source") |
| **Procesador** | STM32F427 (Model T) / STM32F205 (One) |
| **Protección** | Cifrado de flash + contraseña |
| **Conectividad** | USB |
| **Pantalla** | Touch 240x240 / OLED 128x64 |

**Filosofía Trezor:**
- Priorizan transparencia sobre seguridad física extrema
- Dependen de passphrase + cifrado de flash
- Vulnerables a ataques físicos sofisticados (glitching, extracción)
- Código completamente auditable

### 4.3 Comparativa General

| Aspecto | Ledger | Trezor | Este Proyecto (con SE) |
|---------|--------|--------|------------------------|
| **Secure Element** | ✅ ST33 | ❌ Ninguno | ✅ ATECC608/SE050 |
| **Open Source** | ⚠️ Parcial | ✅ Completo | ✅ Hardware/Software |
| **Certificación** | EAL6+ | Ninguna | Depende del SE |
| **Coste BOM** | ~$30-50 | ~$15-25 | ~$12-20 |
| **Anti-tamper** | ✅ Integrado | ⚠️ Software | ✅ Parcial |
| **Recuperabilidad** | Seed phrase | Seed phrase | Seed phrase |

**Elección del ST33 por Ledger:**
Ledger optó por seguridad máxima con ST33 porque:
1. Protección contra ataques físicos avanzados
2. Certificación para uso financiero/institucional
3. Hardware no clonable (patentes + NDA)
4. Gross margin alto para producto premium

**Limitación:** El ST33 no está disponible para proyectos DIY/pocos volumen.

---

## 5. Recomendación Final

### 5.1 Opción Balance (Seguridad/Coste) ⭐

**Recomendación: ATECC608B**

| Aspecto | Justificación |
|---------|---------------|
| **Coste** | ~$3-5 unitario, ideal para producción |
| **Librerías** | Muy maduras (esp-cryptoauthlib) |
| **Integración** | Ejemplos existentes con ESP32 |
| **Características** | ECDSA full, AES-128, random gen |
| **Seguridad** | Claves nunca salen del chip |
| **Certificación** | FIPS validado |

**Configuración sugerida:**
```c
// Slot configuration
Slot 0: Master private key (nunca exportable)
Slot 1-2: Derived keys temporales
Slot 3: Secure counter for transactions
Slot 4-6: Certificates/attestation
Slot 15: Random seed storage
```

**Medidas físicas recomendadas:**
- Epoxy negro conductivo en zona de microcontrolador
- 1-2 switches de tamper en carcasa
- Simple PCB de 2 capas con mesh ground

**Coste total BOM de seguridad:** ~$8-12

### 5.2 Opción Máxima Seguridad

**Recomendación: SE050 + Medidas físicas completas**

| Aspecto | Justificación |
|---------|---------------|
| **SE050** | EAL6+, algoritmos modernos |
| **Ed25519** | Mejor que ECDSA para algunas chains |
| **Memoria** | Soporte para múltiples wallets/apps |
| **Futuro** | TLS 1.3 nativo, criptografía PQC ready |

**Medidas físicas:**
- Cage de Faraday completa + gaskets
- Epoxy negro + mesh conductor en PCB
- 3+ sensores de tamper (case, luz, movimiento)
- Borrado automático en detección
- Case ultrasónicamente sellado

**Coste total BOM de seguridad:** ~$15-25

### 5.3 Justificación Técnica

**¿Por qué ATECC608B como balance?**

1. **Amplitud de verificación:** Usado por miles de dispositivos IoT, bien probado en campo
2. **Integración ESP32:** Espressif proporciona librerías oficiales
3. **Coste escalable:** Si el proyecto crece, el coste por unidad sigue bajo
4. **Seguridad adecuada:** Para un wallet personal/hobbiest, protege contra:
   - Extracción de firmware
   - Clonación simple
   - Ataques de malware (claves nunca en RAM)
   - Robo físico casual (sin equipo especializado)

**¿Cuándo elegir SE050?**
- Si el wallet almacena >$10K típicamente
- Si requiere certificación formal
- Si se planea producción comercial a gran escala
- Si se necesita soporte para múltiples algoritmos

**¿Cuándo NO usar SE externo?**
- Prototipo inicial funcional (usar ESP32 puro)
- Proyecto educativo (añade complejidad innecesaria)
- Dispositivo de testnet únicamente

### 5.4 Roadmap Sugerido

| Fase | Hardware | Justificación |
|------|----------|---------------|
| **v0.1** | ESP32-S3 solo | Validación de concepto web3 BLE |
| **v0.5** | ESP32-S3 + ATECC608 | Seguridad básica para uso personal |
| **v1.0** | ESP32-S3 + ATECC608B + tamper sensors | Producto final balanceado |
| **v1.5** | ESP32-S3 + SE050 + Full tamper | Versión "Pro" (si hay demanda) |

---

## 6. Referencias

### Datasheets
- [ATECC608A Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/ATECC608A-CryptoAuthentication-Device-Summary-DataSheet-DS40001977B.pdf)
- [ATECC608B Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/ATECC608B-CryptoAuthentication-Device-Data-Sheet-DS40002230A.pdf)
- [SE050 Datasheet](https://www.nxp.com/docs/en/data-sheet/SE050-DATASHEET.pdf)
- [STSAFE-A110 Datasheet](https://www.st.com/resource/en/datasheet/stsafe-a110.pdf)
- [ESP32-S3 Security](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/security/security.html)

### Librerías
- [esp-cryptoauthlib](https://github.com/espressif/esp-cryptoauthlib)
- [cryptoauthlib](https://github.com/MicrochipTech/cryptoauthlib)
- [NXP Plug & Trust](https://github.com/NXP/plug-and-trust)

### Documentación Ledger/Trezor
- [Ledger Architecture](https://www.ledger.com/academy/security/architecture)
- [Trezor Security](https://trezor.io/security/)

---

*Documento generado: 2025-06-06*
*Versión: 1.0*
*Autor: Investigador Subagente*