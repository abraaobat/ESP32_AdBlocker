# ESP32 AdBlock Gateway

**Bloqueio de anúncios em nível de DNS no ESP32, com dois modos de implantação: DHCP fornecido pelo roteador ou DHCP + DNS + gateway + NAPT fornecidos pelo próprio ESP32 em uma rede Wi-Fi protegida.**

> Este é um fork modificado de [`s60sc/ESP32_AdBlocker`](https://github.com/s60sc/ESP32_AdBlocker), baseado na versão 3.4. O projeto original fornece o DNS sinkhole. Este fork adiciona o modo opcional de SoftAP protegido/gateway, recuperação do uplink, tratamento seguro de segredos locais e diagnósticos extras de Wi-Fi.

**Idiomas:** [English](README.md) · Português (Brasil) · [Español](README.es.md) · [Français](README.fr.md) · [简体中文](README.zh-CN.md)

## O que há de novo neste fork?

O ESP32_AdBlocker original funciona principalmente como um DNS sinkhole: os clientes usam o ESP32 como servidor DNS e domínios bloqueados resolvem para `0.0.0.0`.

Este fork mantém esse modo e acrescenta uma segunda opção: o ESP32 também pode criar uma rede Wi-Fi separada e protegida. Os clientes conectados recebem endereço IP do ESP32, usam o ESP32 como gateway e DNS, e acessam a Internet por NAPT usando a conexão Wi-Fi de saída do próprio ESP32.

Não se trata de um novo protocolo de DNS ou roteamento. É um novo modo de implantação que combina recursos já existentes do ESP32 em um gateway compacto com bloqueio de anúncios.

## Escolha o modo de rede

| | Modo 1 — DHCP do roteador | Modo 2 — DHCP no ESP32 |
|---|---|---|
| Servidor DHCP | Roteador existente | ESP32 |
| Gateway padrão | Roteador | ESP32 (`192.168.4.1`) |
| DNS | ESP32 | ESP32 (`192.168.4.1`) |
| NAT | Roteador | NAPT do ESP32 → roteador |
| Wi-Fi dos clientes | Wi-Fi normal | SoftAP protegido do ESP32 |
| Alteração no roteador | Normalmente configurar DNS | Normalmente nenhuma |
| Melhor aplicação | Roteador aceita DNS customizado | Roteador da operadora bloqueia DNS/DHCP |

### Modo 1 — o roteador fornece DHCP

```text
Internet
   |
Roteador / DHCP / Gateway
   |
   +---- ESP32 AdBlocker (DNS)
   |
   +---- Celular
   +---- Notebook
   +---- TV / IoT
```

O roteador continua sendo o servidor DHCP e gateway. O ESP32 cuida apenas da filtragem DNS.

Exemplo de informações entregues pelo DHCP do roteador:

```text
IP do cliente: 192.168.1.x
Gateway:       192.168.1.1
DNS:           192.168.1.95   <- ESP32
```

Use este modo quando o roteador permitir definir o DNS da rede local. É o modo de melhor desempenho, pois o tráfego normal de Internet não precisa atravessar o ESP32.

Se o roteador não permitir anunciar um DNS personalizado, você pode configurar o DNS manualmente em cada cliente ou usar o Modo 2.

### Modo 2 — o ESP32 fornece DHCP, DNS e gateway

```text
Internet
   |
Roteador / Wi-Fi existente
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
   +---- Celular     192.168.4.10+
   +---- Notebook
   +---- TV / IoT
```

Os clientes recebem automaticamente:

```text
IP:      192.168.4.x
Gateway: 192.168.4.1
DNS:     192.168.4.1
```

O ESP32 permanece conectado ao Wi-Fi normal como estação e, ao mesmo tempo, cria outra rede Wi-Fi para os clientes protegidos. O tráfego de Internet é encaminhado por NAPT e as consultas DNS são filtradas localmente.

Este modo é especialmente útil quando o roteador da operadora não permite alterar o DNS distribuído via DHCP.

> Não use o ESP32 como um segundo servidor DHCP na mesma LAN do roteador. Neste projeto, o DHCP do ESP32 pertence à sub-rede separada do SoftAP. Evite também que a rede de saída use a mesma faixa `192.168.4.0/24`.

## Situações em que se aplica

- Roteador fornecido pela operadora com DNS bloqueado ou não configurável.
- Rede doméstica com bloqueio de anúncios sem Raspberry Pi ou servidor dedicado.
- Rede separada para Smart TVs, streaming e dispositivos IoT.
- Rede temporária para viagem, laboratório ou demonstração.
- Rede de família ou convidados em que a filtragem DNS deve ser automática.
- Estudo de DNS sinkhole, SoftAP, DHCP e NAPT no ESP32.

## O que consegue e o que não consegue bloquear

A filtragem DNS funciona bem para domínios dedicados a publicidade, rastreamento e telemetria. Ela não bloqueia de forma confiável anúncios servidos pelo mesmo domínio do conteúdo desejado. O YouTube é um exemplo típico de limitação do bloqueio apenas por DNS.

Clientes também podem contornar o DNS local usando DNS criptografado, como DoH/DoT. Desative **DNS Seguro / Secure DNS** no navegador quando quiser que as consultas passem pelo ESP32.

O modo gateway protegido está atualmente focado em IPv4. Desvios por IPv6 devem ser avaliados separadamente em cada ambiente.

## Hardware

Recomendado:

- ESP32-S3 com PSRAM.
- 8 MB de PSRAM para listas de bloqueio grandes atuais.

Hardware usado na validação deste fork:

- ESP32-S3 N16R8
- 16 MB de flash
- 8 MB de PSRAM OPI
- Arduino-ESP32 3.3.11

## Instalação no Arduino IDE

### 1. Instale o core ESP32

No Arduino IDE, instale **esp32 by Espressif Systems**.

Este fork foi validado com a versão **3.3.11**. O projeto upstream documenta como mínimo o Arduino-ESP32 3.1.1.

### 2. Clone ou baixe o repositório

```bash
git clone https://github.com/abraaobat/ESP32_AdBlocker.git
cd ESP32_AdBlocker
```

Abra `ESP32_AdBlocker.ino` no Arduino IDE.

### 3. Configuração recomendada para ESP32-S3 N16R8

```text
Board:            ESP32S3 Dev Module
CPU Frequency:    240 MHz
Flash Size:       16MB (128Mb)
Flash Mode:       QIO
PSRAM:            OPI PSRAM
Partition Scheme: 8M with spiffs
```

Em atualizações normais, mantenha **Erase All Flash Before Sketch Upload** desativado para preservar configurações e dados em cache.

### 4. Primeiro boot / configuração do Wi-Fi de saída

Na primeira instalação, o projeto original cria um AP de configuração semelhante a:

```text
ESP32_AdBlocker_...
```

Conecte-se a ele e abra:

```text
http://192.168.4.1
```

Informe o SSID e a senha do Wi-Fi principal. Após reiniciar, o ESP32 deverá conectar à rede e baixar/processar a blocklist.

## Configurar o Modo 1 — DHCP do roteador

1. Reserve um IPv4 fixo para o ESP32 no DHCP do roteador ou configure IP estático.
2. Configure o DNS da LAN/DHCP do roteador para apontar para o IP do ESP32.
3. Reconecte os clientes ou renove o lease DHCP.
4. Verifique se os clientes receberam o ESP32 como DNS.

Exemplo:

```text
Roteador: 192.168.1.1
ESP32:    192.168.1.95
DNS entregue aos clientes: 192.168.1.95
```

Se o roteador não permitir alterar o DNS da LAN, use o Modo 2.

## Configurar o Modo 2 — DHCP/Gateway no ESP32

A senha do AP protegido **não fica armazenada no Git**.

### 1. Crie o arquivo local de segredos

```bash
cp natSecrets.example.h natSecrets.h
```

Edite `natSecrets.h`:

```cpp
#pragma once

#define PROTECTED_AP_SSID "ESP32-AdBlock"
static const char* PROTECTED_AP_PASS = "CHANGE_ME";
```

Troque `CHANGE_ME` por uma senha privada de 8 a 63 caracteres. O SSID também pode ser alterado.

`natSecrets.h` está no `.gitignore`. Nunca faça commit desse arquivo.

Um clone novo continua compilando sem esse arquivo; nesse caso, o SoftAP protegido permanece desativado.

### 2. Compile e envie

Compile e faça o upload normalmente. Depois que o ESP32 conectar ao Wi-Fi de saída, a rede protegida deve iniciar.

Rede padrão:

```text
SSID:        ESP32-AdBlock
Gateway:     192.168.4.1
DNS:         192.168.4.1
Início DHCP: 192.168.4.10
```

### 3. Conecte um cliente

Conecte um celular, notebook, tablet ou outro dispositivo ao SSID protegido. Ele deverá receber automaticamente um endereço `192.168.4.x`.

## Validação

No macOS/Linux, conectado ao AP protegido:

```bash
ipconfig getifaddr en0                         # macOS
route -n get default | grep gateway           # macOS
dig google.com | grep SERVER
dig doubleclick.net A +short
ping -c 3 1.1.1.1
```

Comportamento esperado:

```text
Cliente:  192.168.4.x
Gateway:  192.168.4.1
DNS:      192.168.4.1
Domínio bloqueado: 0.0.0.0
Internet: acessível
```

## Recuperação do uplink

Se o Wi-Fi de saída cair depois que o AP protegido já estiver ativo, o fork mantém a configuração do AP. Quando a estação reconecta, o firmware restaura a rota padrão pela STA e reaplica o NAPT automaticamente.

Esse caminho de recuperação foi validado em hardware ESP32-S3 com queda e retorno reais do uplink.

Limitação atual: se o dispositivo inicializar sem o Wi-Fi de saída disponível, o AP protegido ainda não funciona como um roteador offline totalmente independente.

## Segurança

- A senha real do Wi-Fi protegido deve existir apenas no `natSecrets.h` local.
- `natSecrets.h` é ignorado pelo Git.
- O firmware não ativa o modo protegido se a senha estiver ausente, continuar como `CHANGE_ME` ou tiver tamanho inválido.
- Nunca publique credenciais em commits, capturas de tela ou logs.
- Bloqueio DNS não substitui firewall, segurança de endpoint ou controle parental completo.

## Créditos e licença

Este projeto deriva do **ESP32_AdBlocker** de `s60sc`. O DNS sinkhole e a aplicação original são trabalho upstream; o modo gateway protegido e sua integração são modificações deste fork.

Licenciado sob a **GNU Affero General Public License v3.0 (AGPL-3.0)**, em conformidade com o projeto upstream. Consulte [`LICENSE`](LICENSE).
