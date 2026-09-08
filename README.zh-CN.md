# ESP32 AdBlock Gateway

**基于 ESP32 的 DNS 级广告拦截，支持两种部署方式：由现有路由器提供 DHCP，或由 ESP32 自己提供 DHCP + DNS + 网关 + NAPT，并创建独立的受保护 Wi-Fi。**

> 本项目是 [`s60sc/ESP32_AdBlocker`](https://github.com/s60sc/ESP32_AdBlocker) 的修改版 fork，基于 v3.4。原项目提供 DNS sinkhole 功能；本 fork 新增可选的受保护 SoftAP/网关模式、上游 Wi-Fi 恢复、更加安全的本地密钥处理，以及额外的 Wi-Fi 诊断信息。

**语言：** [English](README.md) · [Português (Brasil)](README.pt-BR.md) · [Español](README.es.md) · [Français](README.fr.md) · 简体中文

## 这个 fork 新增了什么？

原版 ESP32_AdBlocker 主要作为 DNS sinkhole：客户端把 ESP32 当作 DNS 服务器，被屏蔽的域名会解析为 `0.0.0.0`。

本 fork 保留了这种模式，并增加第二种方式：ESP32 还可以创建一个独立的受保护 Wi-Fi。连接到该网络的客户端由 ESP32 分配 IP 地址，并把 ESP32 同时作为默认网关和 DNS 服务器。Internet 流量通过 ESP32 的上游 Wi-Fi 连接，经 NAPT 转发。

这不是新的 DNS 或路由协议，而是把 ESP32 已有的网络功能组合成一种新的部署方式：一个小型的广告过滤网关。

## 选择网络模式

| | 模式 1 — 路由器提供 DHCP | 模式 2 — ESP32 提供 DHCP |
|---|---|---|
| DHCP 服务器 | 现有路由器 | ESP32 |
| 默认网关 | 路由器 | ESP32 (`192.168.4.1`) |
| DNS | ESP32 | ESP32 (`192.168.4.1`) |
| NAT | 路由器 | ESP32 NAPT → 上游路由器 |
| 客户端 Wi-Fi | 原有 Wi-Fi | ESP32 受保护 SoftAP |
| 是否修改路由器 | 通常需要设置自定义 DNS | 通常不需要 |
| 适合场景 | 路由器允许设置 DNS | ISP 路由器锁定 DNS/DHCP 设置 |

### 模式 1 — 路由器提供 DHCP

```text
Internet
   |
路由器 / DHCP / 网关
   |
   +---- ESP32 AdBlocker (DNS)
   |
   +---- 手机
   +---- 笔记本电脑
   +---- 电视 / IoT
```

路由器仍然负责 DHCP 和默认网关，ESP32 只负责 DNS 过滤。

路由器 DHCP 下发信息示例：

```text
客户端 IP: 192.168.1.x
网关:      192.168.1.1
DNS:       192.168.1.95   <- ESP32
```

如果你的路由器允许为 LAN 设置自定义 DNS，推荐使用这种模式。它的性能最好，因为正常 Internet 流量不会经过 ESP32，只有 DNS 查询发送到 ESP32。

如果路由器不允许修改 DHCP 下发的 DNS，可以在每台客户端上手动设置 DNS，或者使用模式 2。

### 模式 2 — ESP32 提供 DHCP、DNS 和网关

```text
Internet
   |
现有路由器 / Wi-Fi
   |
   | Wi-Fi STA
   v
ESP32
+------------------------+
| DNS sinkhole           |
| DHCP                   |
| IPv4 网关              |
| NAPT                   |
+------------------------+
   |
   | 受保护 SoftAP
   v
ESP32-AdBlock
192.168.4.1
   |
   +---- 手机       192.168.4.10+
   +---- 笔记本电脑
   +---- 电视 / IoT
```

客户端会自动获得：

```text
IP:      192.168.4.x
网关:    192.168.4.1
DNS:     192.168.4.1
```

ESP32 一边作为 STA 连接到原有 Wi-Fi，一边创建独立的受保护 Wi-Fi。Internet 流量通过 NAPT 转发，DNS 查询则由 ESP32 本地过滤。

当 ISP 提供的路由器不允许修改 DHCP 中的 DNS 设置时，这种模式尤其有用。

> 不要让 ESP32 和路由器在同一个 LAN 中同时作为 DHCP 服务器。本项目中，ESP32 的 DHCP 只服务于独立的 SoftAP 子网。同时应避免上游网络本身使用 `192.168.4.0/24`，以免发生子网重叠。

## 适用场景

- ISP 提供的路由器无法修改 DNS。
- 家庭网络中希望进行 DNS 级广告过滤，但不想使用 Raspberry Pi 或专用服务器。
- 为智能电视、流媒体设备和 IoT 设备创建独立网络。
- 旅行、实验室、演示等临时网络。
- 家庭/访客网络，希望设备连接后自动使用过滤 DNS。
- 学习 ESP32 上的 DNS sinkhole、SoftAP、DHCP 和 NAPT。

## 能拦截什么，不能拦截什么

DNS 过滤适合屏蔽专门用于广告、追踪和遥测的域名。但如果广告和正常内容来自同一个域名，就无法可靠地只通过 DNS 屏蔽广告。YouTube 是 DNS-only 过滤的典型限制案例。

客户端也可能通过 DoH/DoT 等加密 DNS 绕过本地 DNS。如果希望 DNS 查询经过 ESP32，请关闭浏览器中的 **Secure DNS / 安全 DNS**。

目前受保护网关模式主要面向 IPv4。IPv6 造成的 DNS/路由绕过需要在具体环境中单独评估。

## 硬件

推荐：

- 带 PSRAM 的 ESP32-S3。
- 对于当前较大的 blocklist，推荐 8 MB PSRAM。

本 fork 的验证硬件：

- ESP32-S3 N16R8
- 16 MB Flash
- 8 MB OPI PSRAM
- Arduino-ESP32 3.3.11

## Arduino IDE 安装

### 1. 安装 ESP32 Core

在 Arduino IDE 中安装 **esp32 by Espressif Systems**。

本 fork 已使用 **3.3.11** 验证。原项目文档中给出的最低 Arduino-ESP32 版本为 3.1.1。

### 2. Clone 或下载仓库

```bash
git clone https://github.com/abraaobat/ESP32_AdBlocker.git
cd ESP32_AdBlocker
```

在 Arduino IDE 中打开 `ESP32_AdBlocker.ino`。

### 3. ESP32-S3 N16R8 推荐设置

```text
Board:            ESP32S3 Dev Module
CPU Frequency:    240 MHz
Flash Size:       16MB (128Mb)
Flash Mode:       QIO
PSRAM:            OPI PSRAM
Partition Scheme: 8M with spiffs
```

普通固件升级时，建议关闭 **Erase All Flash Before Sketch Upload**，以保留配置和缓存数据。

### 4. 首次启动 / 配置上游 Wi-Fi

首次安装时，原项目会创建类似下面的配置 AP：

```text
ESP32_AdBlocker_...
```

连接后打开：

```text
http://192.168.4.1
```

输入主 Wi-Fi 的 SSID 和密码。重启后，ESP32 应连接到上游网络，并下载和处理 blocklist。

## 配置模式 1 — 路由器 DHCP

1. 在路由器中为 ESP32 创建 DHCP 固定租约，或者配置静态 IPv4 地址。
2. 把路由器 LAN/DHCP 中的 DNS 设置为 ESP32 的 IP 地址。
3. 让客户端重新连接，或者更新 DHCP lease。
4. 确认客户端获得的 DNS 是 ESP32。

示例：

```text
路由器: 192.168.1.1
ESP32:  192.168.1.95
客户端获得的 DNS: 192.168.1.95
```

如果路由器不允许修改 LAN DNS，请使用模式 2。

## 配置模式 2 — ESP32 DHCP/网关

受保护 AP 的真实密码**不会存储在 Git 中**。

### 1. 创建本地 secrets 文件

```bash
cp natSecrets.example.h natSecrets.h
```

编辑 `natSecrets.h`：

```cpp
#pragma once

#define PROTECTED_AP_SSID "ESP32-AdBlock"
static const char* PROTECTED_AP_PASS = "CHANGE_ME";
```

把 `CHANGE_ME` 替换为 8 到 63 个字符的私有密码。也可以修改 SSID。

`natSecrets.h` 已被 `.gitignore` 忽略，请不要提交这个文件。

全新 clone 即使没有该文件也可以编译；此时受保护 SoftAP 会保持禁用。

### 2. 编译并烧录

正常编译和上传。ESP32 成功连接上游 Wi-Fi 后，应启动受保护网络。

默认网络参数：

```text
SSID:      ESP32-AdBlock
网关:      192.168.4.1
DNS:       192.168.4.1
DHCP 起始: 192.168.4.10
```

### 3. 连接客户端

让手机、笔记本、平板或其他设备连接受保护 SSID。设备应该自动获得 `192.168.4.x` 地址。

## 验证

macOS/Linux 客户端连接到受保护 AP 后：

```bash
ipconfig getifaddr en0                         # macOS
route -n get default | grep gateway           # macOS
dig google.com | grep SERVER
dig doubleclick.net A +short
ping -c 3 1.1.1.1
```

预期结果：

```text
客户端:  192.168.4.x
网关:    192.168.4.1
DNS:     192.168.4.1
被屏蔽域名: 0.0.0.0
Internet: 可访问
```

## 上游 Wi-Fi 恢复

如果受保护 AP 已经启动后，上游 Wi-Fi 发生中断，本 fork 会保留 SoftAP 配置。当 STA 重新连接后，固件会恢复 STA 默认路由，并自动重新启用 NAPT。

该恢复流程已经在 ESP32-S3 硬件上通过真实的上游断开/恢复测试。

当前限制：如果设备开机时上游 Wi-Fi 就不可用，受保护 AP 尚未设计成完全独立的离线路由器模式。

## 安全

- 真实的受保护 Wi-Fi 密码只能放在本地 `natSecrets.h` 中。
- `natSecrets.h` 已被 Git 忽略。
- 当密码不存在、仍是 `CHANGE_ME` 或长度无效时，固件不会启用受保护模式。
- 不要在 commit、截图或日志中公开凭据。
- DNS 过滤不能替代防火墙、终端安全或完整的家长控制方案。

## 致谢与许可证

本项目衍生自 `s60sc` 的 **ESP32_AdBlocker**。DNS sinkhole 和原始应用属于 upstream 工作；受保护网关模式及其集成属于本 fork 的修改。

本项目继续使用 **GNU Affero General Public License v3.0 (AGPL-3.0)**。详见 [`LICENSE`](LICENSE)。
