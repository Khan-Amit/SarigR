# SarigR®
Test software 

# ❤️ SARIGR™ – Sovereign Hawala Wallet & Vintage Sweeper

**Copyright © 2026 Seliim Ahmed. All rights reserved.**

---

# 💙 SARIGR™ – Sovereign Hawala Wallet & Vintage Sweeper

**Copyright © 2026 Seliim Ahmed. All rights reserved.**

---

## 🛡️ LEGAL NOTICE

This software is the **exclusive intellectual property** of **Seliim Ahmed**.

- **Read-Only Access:** You are permitted to view and study the source code for reference only.
- **No Use Without Permission:** Any use, compilation, modification, distribution, or integration into other projects is **strictly forbidden** without prior written permission from Seliim Ahmed.
- **Legal Enforcement:** Unauthorized use, alteration, abomination, or misrepresentation of this software will be met with legal proceedings under applicable intellectual property laws.

For licensing, inquiries, or written permission:
📧 **amit.khanna.1082@gmail.com**

---

## 📖 About SarigR™

SarigR™ is a sovereign, offline-first cryptocurrency wallet and vintage UTXO sweeper. It features:

- **Enigma Vault** – AES-256-GCM + rotor-scrambled encryption.
- **Hawala Settlement** – Offline, peer-to-peer value transfer via signed tokens.
- **SHA-256 Sweeper** – Vintage block hunting for lost UTXOs (2009–2013 era).
- **Live Mining** – Integrated with XMRig for Monero (XMR) mining.
- **Web Dashboard** – 💙 Blue-themed real-time interface served locally.

---

## 🏗️ Architecture

| Component | Language | Role |
| :--- | :--- | :--- |
| `NetworkClient` | C++17 | TCP transport (raw socket) |
| `KhutiEnigma` | C++17 | Encryption vault |
| `KhutiWallet` | C++17 | Key management, Hawala, SHA-256 sweeper, miner launcher |
| `KhutiWebServer` | C++17 | Embedded HTTP server + SSE streaming |
| `index.html` | HTML/CSS/JS | Front-end dashboard |

---

## 🚀 Building (For Reference Only)

**You are not authorized to build or run this software without explicit written permission from Seliim Ahmed.**

If you have received permission:

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install g++ libssl-dev nlohmann-json3-dev

# Download XMRig (static build)
wget https://github.com/xmrig/xmrig/releases/download/v6.21.3/xmrig-6.21.3-linux-static-x64.tar.gz
tar -xzf xmrig-6.21.3-linux-static-x64.tar.gz
mv xmrig-6.21.3/xmrig ./xmrig
chmod +x xmrig

# Edit KhutiWallet.cpp – replace YOUR_WALLET_ADDRESS with your XMR address.

# Compile
g++ -std=c++17 -O3 -pthread *.cpp -lssl -lcrypto -o sarig

# Run
./sarig /path/to/your/vault.khuti
