# ESP32 AdBlock Gateway

**Bloqueo de anuncios a nivel DNS en ESP32, con dos modos de despliegue: DHCP proporcionado por el router o DHCP + DNS + gateway + NAPT proporcionados directamente por el ESP32 en una red Wi-Fi protegida.**

> Este es un fork modificado de [`s60sc/ESP32_AdBlocker`](https://github.com/s60sc/ESP32_AdBlocker), basado en la versión 3.4. El proyecto original aporta el DNS sinkhole. Este fork añade el modo opcional SoftAP protegido/gateway, recuperación del uplink, gestión segura de secretos locales y diagnósticos Wi-Fi adicionales.

**Idiomas:** [English](README.md) · [Português (Brasil)](README.pt-BR.md) · Español · [Français](README.fr.md) · [简体中文](README.zh-CN.md)

## ¿Qué añade este fork?

El ESP32_AdBlocker original funciona principalmente como un DNS sinkhole: los clientes usan el ESP32 como servidor DNS y los dominios bloqueados resuelven a `0.0.0.0`.

Este fork mantiene ese modo y añade una segunda opción: el ESP32 también puede crear una red Wi-Fi separada y protegida. Los clientes conectados reciben una dirección IP del ESP32, usan el ESP32 como gateway y DNS, y acceden a Internet mediante NAPT a través de la conexión Wi-Fi de salida del propio ESP32.

No es un protocolo nuevo de DNS o enrutamiento. Es un nuevo modo de despliegue que combina capacidades de red existentes del ESP32 en un gateway compacto con bloqueo de anuncios.

## Elige el modo de red

| | Modo 1 — DHCP del router | Modo 2 — DHCP del ESP32 |
|---|---|---|
| Servidor DHCP | Router existente | ESP32 |
| Gateway | Router | ESP32 (`192.168.4.1`) |
| DNS | ESP32 | ESP32 (`192.168.4.1`) |
| NAT | Router | NAPT del ESP32 → router |
| Wi-Fi de clientes | Wi-Fi normal | SoftAP protegido del ESP32 |
| Cambios en el router | Normalmente configurar DNS | Normalmente ninguno |
| Mejor para | Routers con DNS configurable | Routers del ISP con DNS/DHCP bloqueados |

### Modo 1 — el router proporciona DHCP

```text
Internet
   |
Router / DHCP / Gateway
   |
   +---- ESP32 AdBlocker (DNS)
   |
   +---- Teléfono
   +---- Portátil
   +---- TV / IoT
```

El router sigue siendo el servidor DHCP y el gateway. El ESP32 solo filtra DNS.

Ejemplo de datos DHCP entregados por el router:

```text
IP del cliente: 192.168.1.x
Gateway:        192.168.1.1
DNS:            192.168.1.95   <- ESP32
```

Usa este modo cuando tu router permita definir un DNS para la LAN. Ofrece el mejor rendimiento porque el tráfico normal de Internet no atraviesa el ESP32.

Si el router no permite anunciar un DNS personalizado, puedes configurarlo manualmente en cada cliente o usar el Modo 2.

### Modo 2 — el ESP32 proporciona DHCP, DNS y gateway

```text
Internet
   |
Router / Wi-Fi existente
   |
   | Wi-Fi STA
   v
ESP32
+------------------------+
| DNS sinkhole           |
| DHCP                   |
| Gateway IPv4           |
| NAPT                   |
+------------------------+
   |
   | SoftAP protegido
   v
ESP32-AdBlock
192.168.4.1
   |
   +---- Teléfono    192.168.4.10+
   +---- Portátil
   +---- TV / IoT
```

Los clientes reciben automáticamente:

```text
IP:      192.168.4.x
Gateway: 192.168.4.1
DNS:     192.168.4.1
```

El ESP32 permanece conectado al Wi-Fi normal como estación y, simultáneamente, crea otra red Wi-Fi para clientes protegidos. El tráfico de Internet se reenvía mediante NAPT y las consultas DNS se filtran localmente.

Este modo es especialmente útil cuando el router del proveedor no permite cambiar el DNS distribuido por DHCP.

> No uses el ESP32 como segundo servidor DHCP en la misma LAN que el router. En este proyecto, el DHCP del ESP32 pertenece a la subred separada del SoftAP. Evita también que la red upstream use `192.168.4.0/24`.

## Casos de uso

- Router del ISP con DNS bloqueado o no configurable.
- Red doméstica con bloqueo DNS sin Raspberry Pi ni servidor dedicado.
- Red separada para Smart TV, streaming o IoT.
- Red temporal para viajes, laboratorios o demostraciones.
- Red familiar o de invitados con filtrado DNS automático.
- Aprendizaje de DNS sinkhole, SoftAP, DHCP y NAPT en ESP32.

## Qué puede y qué no puede bloquear

El filtrado DNS funciona bien con dominios dedicados a publicidad, rastreo y telemetría. No puede bloquear de forma fiable anuncios servidos desde el mismo dominio que el contenido deseado. YouTube es un ejemplo típico de esta limitación.

Los clientes también pueden eludir el DNS local mediante DNS cifrado, como DoH/DoT. Desactiva **Secure DNS / DNS seguro** en el navegador si quieres que las consultas pasen por el ESP32.

El modo gateway protegido está actualmente centrado en IPv4. El posible bypass por IPv6 debe evaluarse por separado en cada entorno.

## Hardware

Recomendado:

- ESP32-S3 con PSRAM.
- 8 MB de PSRAM para listas de bloqueo grandes.

Hardware usado para validar este fork:

- ESP32-S3 N16R8
- 16 MB flash
- 8 MB OPI PSRAM
- Arduino-ESP32 3.3.11

## Instalación en Arduino IDE

### 1. Instala el core de ESP32

En Arduino IDE instala **esp32 by Espressif Systems**.

Este fork fue validado con **3.3.11**. El proyecto upstream documenta Arduino-ESP32 3.1.1 como versión mínima.

### 2. Clona o descarga el repositorio

```bash
git clone https://github.com/abraaobat/ESP32_AdBlocker.git
cd ESP32_AdBlocker
```

Abre `ESP32_AdBlocker.ino` en Arduino IDE.

### 3. Ajustes recomendados para ESP32-S3 N16R8

```text
Board:            ESP32S3 Dev Module
CPU Frequency:    240 MHz
Flash Size:       16MB (128Mb)
Flash Mode:       QIO
PSRAM:            OPI PSRAM
Partition Scheme: 8M with spiffs
```

Para actualizaciones normales, mantén **Erase All Flash Before Sketch Upload** desactivado para conservar la configuración y los datos almacenados.

### 4. Primer arranque / Wi-Fi upstream

En la primera instalación, el proyecto original crea un AP de configuración similar a:

```text
ESP32_AdBlocker_...
```

Conéctate y abre:

```text
http://192.168.4.1
```

Introduce el SSID y la contraseña del Wi-Fi principal. Tras reiniciar, el ESP32 debería conectarse a la red y descargar/procesar la blocklist.

## Configurar Modo 1 — DHCP del router

1. Asigna al ESP32 una IPv4 estable mediante reserva DHCP o IP estática.
2. Configura el DNS de LAN/DHCP del router para apuntar a la IP del ESP32.
3. Reconecta los clientes o renueva sus leases DHCP.
4. Verifica que los clientes hayan recibido el ESP32 como DNS.

Ejemplo:

```text
Router: 192.168.1.1
ESP32:  192.168.1.95
DNS entregado a clientes: 192.168.1.95
```

Si el router no permite modificar el DNS de la LAN, usa el Modo 2.

## Configurar Modo 2 — DHCP/Gateway del ESP32

La contraseña del AP protegido **no se almacena en Git**.

### 1. Crea el archivo local de secretos

```bash
cp natSecrets.example.h natSecrets.h
```

Edita `natSecrets.h`:

```cpp
#pragma once

#define PROTECTED_AP_SSID "ESP32-AdBlock"
static const char* PROTECTED_AP_PASS = "CHANGE_ME";
```

Sustituye `CHANGE_ME` por una contraseña privada de 8 a 63 caracteres. También puedes cambiar el SSID.

`natSecrets.h` está ignorado por `.gitignore`. Nunca lo subas al repositorio.

Un clon nuevo sigue compilando sin este archivo; en ese caso el SoftAP protegido permanece desactivado.

### 2. Compila y carga

Compila y carga normalmente. Después de que el ESP32 se conecte al Wi-Fi upstream, debería iniciar la red protegida.

Red predeterminada:

```text
SSID:        ESP32-AdBlock
Gateway:     192.168.4.1
DNS:         192.168.4.1
Inicio DHCP: 192.168.4.10
```

### 3. Conecta un cliente

Conecta un teléfono, portátil, tableta u otro dispositivo al SSID protegido. Debería recibir automáticamente una dirección `192.168.4.x`.

## Validación

En macOS/Linux, conectado al AP protegido:

```bash
ipconfig getifaddr en0                         # macOS
route -n get default | grep gateway           # macOS
dig google.com | grep SERVER
dig doubleclick.net A +short
ping -c 3 1.1.1.1
```

Resultado esperado:

```text
Cliente:  192.168.4.x
Gateway:  192.168.4.1
DNS:      192.168.4.1
Dominio bloqueado: 0.0.0.0
Internet: accesible
```

## Recuperación del uplink

Si el Wi-Fi upstream se pierde después de que el AP protegido ya esté activo, el fork conserva la configuración del AP. Cuando la estación vuelve a conectarse, el firmware restaura la ruta predeterminada por STA y vuelve a aplicar NAPT automáticamente.

Este comportamiento fue validado en hardware ESP32-S3 con una caída y recuperación reales del uplink.

Limitación actual: si el dispositivo arranca sin Wi-Fi upstream disponible, el AP protegido todavía no está diseñado como un router offline completamente independiente.

## Seguridad

- La contraseña real del Wi-Fi protegido debe existir solo en `natSecrets.h` local.
- `natSecrets.h` está ignorado por Git.
- El firmware no activa el modo protegido si falta la contraseña, sigue siendo `CHANGE_ME` o tiene una longitud inválida.
- Nunca publiques credenciales en commits, capturas o logs.
- El bloqueo DNS no sustituye a un firewall, seguridad de endpoint ni control parental completo.

## Créditos y licencia

Este proyecto deriva de **ESP32_AdBlocker** de `s60sc`. El DNS sinkhole y la aplicación original son trabajo upstream; el modo gateway protegido y su integración son modificaciones de este fork.

Licenciado bajo **GNU Affero General Public License v3.0 (AGPL-3.0)**, de acuerdo con el proyecto upstream. Consulta [`LICENSE`](LICENSE).
