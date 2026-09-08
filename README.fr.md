# ESP32 AdBlock Gateway

**Blocage publicitaire au niveau DNS sur ESP32, avec deux modes de déploiement : DHCP fourni par le routeur, ou DHCP + DNS + passerelle + NAPT fournis directement par l’ESP32 via un réseau Wi-Fi protégé.**

> Ce projet est un fork modifié de [`s60sc/ESP32_AdBlocker`](https://github.com/s60sc/ESP32_AdBlocker), basé sur la version 3.4. Le projet d’origine fournit le DNS sinkhole. Ce fork ajoute un mode SoftAP protégé/passerelle, la récupération de l’uplink, une gestion plus sûre des secrets locaux et des diagnostics Wi-Fi supplémentaires.

**Langues :** [English](README.md) · [Português (Brasil)](README.pt-BR.md) · [Español](README.es.md) · Français · [简体中文](README.zh-CN.md)

## Qu’apporte ce fork ?

Le projet ESP32_AdBlocker d’origine fonctionne principalement comme un DNS sinkhole : les clients utilisent l’ESP32 comme serveur DNS et les domaines bloqués sont résolus vers `0.0.0.0`.

Ce fork conserve ce mode et ajoute une seconde possibilité : l’ESP32 peut aussi créer un réseau Wi-Fi séparé et protégé. Les clients connectés reçoivent leur adresse IP de l’ESP32, utilisent l’ESP32 comme passerelle et serveur DNS, puis accèdent à Internet via NAPT en utilisant la connexion Wi-Fi amont de l’ESP32.

Il ne s’agit pas d’un nouveau protocole DNS ou de routage. C’est un nouveau mode de déploiement qui combine des fonctions réseau existantes de l’ESP32 dans une petite passerelle de blocage publicitaire.

## Choisir le mode réseau

| | Mode 1 — DHCP du routeur | Mode 2 — DHCP de l’ESP32 |
|---|---|---|
| Serveur DHCP | Routeur existant | ESP32 |
| Passerelle | Routeur | ESP32 (`192.168.4.1`) |
| DNS | ESP32 | ESP32 (`192.168.4.1`) |
| NAT | Routeur | NAPT ESP32 → routeur |
| Wi-Fi client | Wi-Fi habituel | SoftAP protégé ESP32 |
| Modification du routeur | Généralement configurer le DNS | Généralement aucune |
| Idéal pour | Routeur avec DNS configurable | Routeur FAI avec DNS/DHCP verrouillé |

### Mode 1 — le routeur fournit le DHCP

```text
Internet
   |
Routeur / DHCP / Passerelle
   |
   +---- ESP32 AdBlocker (DNS)
   |
   +---- Téléphone
   +---- Ordinateur
   +---- TV / IoT
```

Le routeur reste serveur DHCP et passerelle. L’ESP32 ne s’occupe que du filtrage DNS.

Exemple d’informations fournies par le DHCP du routeur :

```text
IP client : 192.168.1.x
Passerelle : 192.168.1.1
DNS :        192.168.1.95   <- ESP32
```

Utilisez ce mode lorsque votre routeur permet de définir un serveur DNS pour le LAN. C’est le mode offrant le meilleur débit, car le trafic Internet normal ne traverse pas l’ESP32.

Si le routeur ne permet pas d’annoncer un DNS personnalisé, vous pouvez configurer le DNS manuellement sur chaque client ou utiliser le Mode 2.

### Mode 2 — l’ESP32 fournit DHCP, DNS et passerelle

```text
Internet
   |
Routeur / Wi-Fi existant
   |
   | Wi-Fi STA
   v
ESP32
+------------------------+
| DNS sinkhole           |
| DHCP                   |
| Passerelle IPv4        |
| NAPT                   |
+------------------------+
   |
   | SoftAP protégé
   v
ESP32-AdBlock
192.168.4.1
   |
   +---- Téléphone   192.168.4.10+
   +---- Ordinateur
   +---- TV / IoT
```

Les clients reçoivent automatiquement :

```text
IP :         192.168.4.x
Passerelle : 192.168.4.1
DNS :        192.168.4.1
```

L’ESP32 reste connecté au Wi-Fi normal en tant que station tout en créant simultanément un second réseau Wi-Fi pour les clients protégés. Le trafic Internet est transmis via NAPT et les requêtes DNS sont filtrées localement.

Ce mode est particulièrement utile lorsque le routeur du FAI ne permet pas de modifier le DNS distribué par DHCP.

> N’utilisez pas l’ESP32 comme deuxième serveur DHCP sur le même LAN que le routeur. Dans ce projet, le DHCP ESP32 appartient au sous-réseau séparé du SoftAP. Évitez aussi que le réseau amont utilise `192.168.4.0/24`.

## Cas d’usage

- Routeur fourni par le FAI avec DNS verrouillé ou non configurable.
- Réseau domestique avec filtrage DNS sans Raspberry Pi ni serveur dédié.
- Réseau séparé pour Smart TV, streaming et objets connectés.
- Réseau temporaire pour voyage, laboratoire ou démonstration.
- Réseau familial ou invité avec filtrage DNS automatique.
- Apprentissage du DNS sinkhole, SoftAP, DHCP et NAPT sur ESP32.

## Ce que le filtrage peut et ne peut pas bloquer

Le filtrage DNS est efficace pour les domaines dédiés à la publicité, au suivi et à la télémétrie. Il ne peut pas bloquer de façon fiable les publicités servies depuis le même domaine que le contenu souhaité. YouTube est un exemple classique des limites d’un blocage uniquement DNS.

Les clients peuvent aussi contourner le DNS local avec du DNS chiffré comme DoH/DoT. Désactivez **Secure DNS / DNS sécurisé** dans le navigateur si vous souhaitez que les requêtes passent par l’ESP32.

Le mode passerelle protégé est actuellement centré sur IPv4. Un éventuel contournement par IPv6 doit être évalué séparément selon l’environnement.

## Matériel

Recommandé :

- ESP32-S3 avec PSRAM.
- 8 Mo de PSRAM pour les listes de blocage volumineuses actuelles.

Matériel utilisé pour valider ce fork :

- ESP32-S3 N16R8
- 16 Mo de flash
- 8 Mo de PSRAM OPI
- Arduino-ESP32 3.3.11

## Installation avec Arduino IDE

### 1. Installer le core ESP32

Dans Arduino IDE, installez **esp32 by Espressif Systems**.

Ce fork a été validé avec la version **3.3.11**. Le projet upstream indique Arduino-ESP32 3.1.1 comme version minimale.

### 2. Cloner ou télécharger le dépôt

```bash
git clone https://github.com/abraaobat/ESP32_AdBlocker.git
cd ESP32_AdBlocker
```

Ouvrez `ESP32_AdBlocker.ino` dans Arduino IDE.

### 3. Réglages recommandés pour ESP32-S3 N16R8

```text
Board:            ESP32S3 Dev Module
CPU Frequency:    240 MHz
Flash Size:       16MB (128Mb)
Flash Mode:       QIO
PSRAM:            OPI PSRAM
Partition Scheme: 8M with spiffs
```

Pour les mises à jour normales, laissez **Erase All Flash Before Sketch Upload** désactivé afin de préserver la configuration et les données en cache.

### 4. Premier démarrage / configuration du Wi-Fi amont

Lors de la première installation, le projet d’origine crée un AP de configuration semblable à :

```text
ESP32_AdBlocker_...
```

Connectez-vous puis ouvrez :

```text
http://192.168.4.1
```

Saisissez le SSID et le mot de passe du Wi-Fi principal. Après redémarrage, l’ESP32 doit se connecter au réseau puis télécharger et traiter la blocklist.

## Configurer le Mode 1 — DHCP du routeur

1. Donnez à l’ESP32 une adresse IPv4 stable via réservation DHCP ou adresse statique.
2. Configurez le DNS LAN/DHCP du routeur pour pointer vers l’adresse IP de l’ESP32.
3. Reconnectez les clients ou renouvelez leur bail DHCP.
4. Vérifiez que les clients utilisent l’ESP32 comme DNS.

Exemple :

```text
Routeur : 192.168.1.1
ESP32 :   192.168.1.95
DNS distribué aux clients : 192.168.1.95
```

Si le routeur ne permet pas de modifier le DNS du LAN, utilisez le Mode 2.

## Configurer le Mode 2 — DHCP/Passerelle sur ESP32

Le mot de passe du point d’accès protégé **n’est pas stocké dans Git**.

### 1. Créer le fichier local de secrets

```bash
cp natSecrets.example.h natSecrets.h
```

Modifiez `natSecrets.h` :

```cpp
#pragma once

#define PROTECTED_AP_SSID "ESP32-AdBlock"
static const char* PROTECTED_AP_PASS = "CHANGE_ME";
```

Remplacez `CHANGE_ME` par un mot de passe privé de 8 à 63 caractères. Le SSID peut également être modifié.

`natSecrets.h` est ignoré par `.gitignore`. Ne le commitez jamais.

Un nouveau clone compile même sans ce fichier ; dans ce cas le SoftAP protégé reste simplement désactivé.

### 2. Compiler et téléverser

Compilez et téléversez normalement. Une fois l’ESP32 connecté au Wi-Fi amont, le réseau protégé devrait démarrer.

Réseau par défaut :

```text
SSID :        ESP32-AdBlock
Passerelle :  192.168.4.1
DNS :         192.168.4.1
Début DHCP :  192.168.4.10
```

### 3. Connecter un client

Connectez un téléphone, ordinateur, tablette ou autre appareil au SSID protégé. Il doit recevoir automatiquement une adresse `192.168.4.x`.

## Validation

Sous macOS/Linux, connecté à l’AP protégé :

```bash
ipconfig getifaddr en0                         # macOS
route -n get default | grep gateway           # macOS
dig google.com | grep SERVER
dig doubleclick.net A +short
ping -c 3 1.1.1.1
```

Résultat attendu :

```text
Client :      192.168.4.x
Passerelle :  192.168.4.1
DNS :         192.168.4.1
Domaine bloqué : 0.0.0.0
Internet : accessible
```

## Récupération de l’uplink

Si le Wi-Fi amont disparaît après l’activation du réseau protégé, le fork conserve la configuration du SoftAP. Lorsque la station se reconnecte, le firmware restaure la route par défaut via STA et réactive automatiquement NAPT.

Ce comportement a été validé sur matériel ESP32-S3 avec une coupure et un retour réels de l’uplink.

Limitation actuelle : si l’appareil démarre alors que le Wi-Fi amont est indisponible, le SoftAP protégé n’est pas encore conçu comme un routeur hors ligne totalement indépendant.

## Sécurité

- Le vrai mot de passe Wi-Fi protégé doit exister uniquement dans `natSecrets.h` local.
- `natSecrets.h` est ignoré par Git.
- Le firmware n’active pas le mode protégé si le mot de passe est absent, encore défini sur `CHANGE_ME` ou de longueur invalide.
- Ne publiez jamais d’identifiants dans des commits, captures ou logs.
- Le filtrage DNS ne remplace pas un pare-feu, la sécurité des terminaux ou un contrôle parental complet.

## Crédits et licence

Ce projet est dérivé de **ESP32_AdBlocker** par `s60sc`. Le DNS sinkhole et l’application originale sont le travail upstream ; le mode passerelle protégé et son intégration sont des modifications de ce fork.

Sous licence **GNU Affero General Public License v3.0 (AGPL-3.0)**, conformément au projet upstream. Voir [`LICENSE`](LICENSE).
