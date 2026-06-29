/*
 * SARIGR – Minimal Working Rig
 * Copyright (c) 2026 Seliim Ahmed
 *
 * Compile: g++ -std=c++17 -O3 -pthread rig.cpp -lssl -lcrypto -o rig
 * Run:     ./rig
 */

#include <openssl/sha.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ---------- Simple TCP client ----------
class NetClient {
    int sock;
public:
    NetClient() : sock(-1) {}
    bool connect(const std::string& host, int port) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return false;
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
        return ::connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0;
    }
    void send(const std::string& msg) {
        ::send(sock, msg.c_str(), msg.size(), 0);
    }
    std::string recv() {
        char buf[4096];
        int n = ::recv(sock, buf, sizeof(buf)-1, 0);
        if (n <= 0) return "";
        buf[n] = '\0';
        return std::string(buf);
    }
    void close() { if (sock >= 0) ::close(sock); sock = -1; }
};

// ---------- Hex to bytes ----------
std::vector<unsigned char> hex2bytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    for (size_t i = 0; i + 1 < hex.length(); i += 2) {
        std::string byteStr = hex.substr(i, 2);
        bytes.push_back((unsigned char)strtol(byteStr.c_str(), nullptr, 16));
    }
    return bytes;
}

// ---------- MAIN ----------
int main() {
    NetClient pool;
    if (!pool.connect("pool.supportxmr.com", 3333)) {
        std::cerr << "Pool connection failed.\n";
        return 1;
    }

    // Login – replace with your real wallet address
    std::string wallet = "YOUR_WALLET_ADDRESS";
    json login = {{"method", "login"}, {"params", {{"login", wallet}}}};
    pool.send(login.dump() + "\n");

    int shares = 0;
    while (true) {
        std::string line = pool.recv();
        if (line.empty()) break;

        // Parse JSON
        json data;
        try { data = json::parse(line); } catch (...) { continue; }

        // Only care about mining.notify
        if (!data.contains("method") || data["method"] != "mining.notify")
            continue;

        auto params = data["params"];
        if (params.size() < 5) continue;

        std::string blob_hex = params[0];
        std::string target_hex = params[4];

        // Convert to binary
        auto header = hex2bytes(blob_hex);
        if (header.size() < 80) continue;

        // Target (simplified)
        unsigned char target[32] = {0};
        auto tbytes = hex2bytes(target_hex);
        for (size_t i = 0; i < tbytes.size() && i < 32; i++)
            target[i] = tbytes[i];

        // Hunt for nonce
        bool found = false;
        uint32_t nonce = 0;
        unsigned char hash[32];

        while (!found && nonce < 0xFFFFFFFF) {
            // Insert nonce into header (little endian)
            header[76] = (nonce >> 0) & 0xFF;
            header[77] = (nonce >> 8) & 0xFF;
            header[78] = (nonce >> 16) & 0xFF;
            header[79] = (nonce >> 24) & 0xFF;

            // Double SHA-256
            SHA256_CTX ctx;
            SHA256_Init(&ctx);
            SHA256_Update(&ctx, header.data(), header.size());
            SHA256_Final(hash, &ctx);
            SHA256_Init(&ctx);
            SHA256_Update(&ctx, hash, 32);
            SHA256_Final(hash, &ctx);

            // Check against target (compare first 4 bytes)
            if (memcmp(hash, target, 4) <= 0) {
                found = true;
                shares++;
                json submit = {{"method", "submit"}, {"params", {{"nonce", nonce}}}};
                pool.send(submit.dump() + "\n");
                std::cout << "✅ Share found! Nonce: " << nonce << " | Shares: " << shares << "\n";
            }
            nonce++;
        }
    }

    pool.close();
    return 0;
}
