# Hardware Wallet ESP32

Hardware wallet air-gapped para Bitcoin basada en ESP32-S3.

## Características

- 🔒 **Air-gapped**: Firma transacciones sin conexión a internet
- 🔐 **Seguridad**: Claves privadas nunca salen del dispositivo
- 📱 **Interfaz**: Display OLED + botones físicos
- 💾 **Almacenamiento**: Soporte microSD para PSBT
- 📷 **Comunicación**: Códigos QR para transferencia
- ⚡ **Estándares**: BIP39/BIP32/BIP44/PSBT

## Hardware Requerido

| Componente | Modelo |
|------------|--------|
| MCU | ESP32-S3-WROOM-1-N8R8 |
| Display | OLED SSD1306 128x64 I2C |
| Almacenamiento | MicroSD + slot SPI |
| Botones | 3x botones táctiles |
| USB | Conexión para power/data |

## Pinout

| ESP32-S3 | Componente | Pin |
|----------|------------|-----|
| GPIO8 | OLED SDA | - |
| GPIO9 | OLED SCL | - |
| GPIO4 | SD CS | - |
| GPIO5 | SD MOSI | - |
| GPIO6 | SD MISO | - |
| GPIO7 | SD CLK | - |
| GPIO10 | BTN UP | - |
| GPIO11 | BTN DOWN | - |
| GPIO12 | BTN CONFIRM | - |

## Compilación

```bash
cd firmware
idf.py menuconfig
idf.py build
idf.py flash
idf.py monitor
```

## Uso

### 1. Crear Wallet

1. Encender dispositivo
2. Seleccionar "Create New Wallet"
3. Anotar seed phrase (12/24 palabras)
4. Confirmar backup

### 2. Firmar Transacción (Air-gapped)

1. Desktop app crea archivo PSBT sin firmar
2. Guardar PSBT en tarjeta SD
3. Insertar SD en wallet
4. Verificar transacción en display
5. Confirmar con botón físico
6. Wallet guarda PSBT firmado en SD
7. Desktop app carga PSBT firmado
8. Broadcast a red Bitcoin

### 3. Mostrar Dirección

1. Seleccionar "Show Address"
2. Display muestra dirección + QR
3. Verificar en external wallet

## Estructura del Proyecto

```
hardware-wallet-esp32/
├── firmware/           # ESP-IDF project
│   ├── main/
│   │   ├── main.c     # Entry point
│   │   ├── crypto/    # BIP39/BIP32/BIP44/PSBT
│   │   ├── drivers/   # OLED, SD, buttons
│   │   └── include/   # Headers
│   └── CMakeLists.txt
├── desktop/           # Python companion
│   ├── wallet_cli.py  # CLI interface
│   └── psbt_builder.py
└── README.md
```

## Seguridad

⚠️ **IMPORTANTE**:
- Este es un proyecto experimental
- Realizar auditoría de seguridad antes de uso en producción
- El ESP32 no tiene secure element dedicado
- Flash encryption habilitado para protección básica
- No usar para cantidades significativas sin revisión

## Licencia

MIT License - Ver LICENSE

## Disclaimer

Este proyecto se proporciona "tal cual" sin garantías de ningún tipo.
El uso es bajo tu propia responsabilidad.
