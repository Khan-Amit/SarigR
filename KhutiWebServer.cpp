#include "KhutiWebServer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <thread>
#include <iostream>
#include <nlohmann/json.hpp>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <cstdio>

std::queue<std::string> minerOutputQueue;
std::mutex queueMutex;
std::condition_variable queueCV;
bool minerRunning = false;

KhutiWebServer::KhutiWebServer(KhutiWallet& w) : wallet(w), server_fd(-1) {}

std::string KhutiWebServer::build_response(const std::string& body, const std::string& content_type) {
    std::ostringstream resp;
    resp << "HTTP/1.1 200 OK\r\n";
    resp << "Content-Type: " << content_type << "\r\n";
    resp << "Content-Length: " << body.size() << "\r\n";
    resp << "Access-Control-Allow-Origin: *\r\n";
    resp << "Connection: close\r\n\r\n";
    resp << body;
    return resp.str();
}

std::string KhutiWebServer::get_embedded_html() {
    std::ifstream file("index.html");
    if (!file.is_open()) {
        return "<h1>💙 SarigR Rig</h1><p>index.html not found. Place it in the same folder.</p>";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void KhutiWebServer::handle_client(int client_fd) {
    char buffer[4096] = {0};
    read(client_fd, buffer, sizeof(buffer) - 1);
    std::string request(buffer);
    std::istringstream req(request);
    std::string method, path, version;
    req >> method >> path >> version;

    std::string body;
    if (method == "POST") {
        size_t pos = request.find("\r\n\r\n");
        if (pos != std::string::npos) {
            body = request.substr(pos + 4);
        }
    }

    std::string response_body, content_type = "text/html";

    if (path == "/" || path == "/index.html") {
        response_body = get_embedded_html();
    }
    else if (path == "/status") {
        nlohmann::json j;
        j["address"] = wallet.getAddress();
        j["balance_onchain"] = wallet.getOnChainBalance();
        j["balance_hawala"] = wallet.getHawalaPending();
        j["total"] = j["balance_onchain"].get<double>() + j["balance_hawala"].get<double>();
        j["block_height"] = wallet.getLastBlockHeight();
        j["hashrate"] = wallet.getSweeperHashRate();
        j["shares"] = wallet.getShares();
        j["pool"] = "Kryptex (UAE)";
        j["status"] = minerRunning ? "Mining" : "Disconnected";
        response_body = j.dump();
        content_type = "application/json";
    }
    else if (path == "/sweep" && method == "POST") {
        std::thread([this]() { wallet.sweep_vintage_utxos(); }).detach();
        response_body = R"({"status":"sweep_started"})";
        content_type = "application/json";
    }
    else if (path == "/hawala/create" && method == "POST") {
        try {
            nlohmann::json req_json = nlohmann::json::parse(body);
            std::string receiver = req_json["receiver"];
            double amount = req_json["amount"];
            uint64_t nonce = (uint64_t)std::time(nullptr);
            std::string token = wallet.createHawalaToken(receiver, amount, nonce);
            nlohmann::json resp; resp["status"] = "created"; resp["token"] = token;
            response_body = resp.dump(); content_type = "application/json";
        } catch (...) {
            nlohmann::json resp; resp["status"] = "error"; resp["reason"] = "Invalid JSON";
            response_body = resp.dump(); content_type = "application/json";
        }
    }
    else if (path == "/hawala/verify" && method == "POST") {
        try {
            nlohmann::json req_json = nlohmann::json::parse(body);
            std::string token = req_json["token"];
            bool accepted = wallet.acceptHawalaToken(token);
            nlohmann::json resp;
            if (accepted) { resp["status"] = "accepted"; resp["amount"] = wallet.getHawalaPending(); }
            else { resp["status"] = "rejected"; resp["reason"] = "Invalid"; }
            response_body = resp.dump(); content_type = "application/json";
        } catch (...) {
            nlohmann::json resp; resp["status"] = "error"; resp["reason"] = "Invalid JSON";
            response_body = resp.dump(); content_type = "application/json";
        }
    }
    else if (path == "/start_rig" && method == "POST") {
        if (!minerRunning) {
            std::thread([this]() { wallet.startMiner(); }).detach();
            response_body = R"({"status":"miner_started"})";
        } else {
            response_body = R"({"status":"already_running"})";
        }
        content_type = "application/json";
    }
    else if (path == "/events") {
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/event-stream\r\n";
        response += "Cache-Control: no-cache\r\n";
        response += "Connection: keep-alive\r\n\r\n";
        send(client_fd, response.c_str(), response.size(), 0);

        while (true) {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCV.wait(lock, []{ return !minerOutputQueue.empty(); });
            while (!minerOutputQueue.empty()) {
                std::string line = minerOutputQueue.front();
                minerOutputQueue.pop();
                lock.unlock();
                std::string sse = "data: " + line + "\n\n";
                send(client_fd, sse.c_str(), sse.size(), 0);
                lock.lock();
            }
        }
        close(client_fd);
        return;
    }
    else {
        response_body = "<h1>💙 SarigR Rig</h1><p>Endpoint not found</p>";
    }

    std::string resp = build_response(response_body, content_type);
    send(client_fd, resp.c_str(), resp.size(), 0);
    close(client_fd);
}

void KhutiWebServer::start(int port) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 10);

    std::cout << "💙 SarigR Web Dashboard: http://localhost:" << port << "\n";

    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client >= 0) {
            std::thread([this, client]() { handle_client(client); }).detach();
        }
    }
}

void KhutiWebServer::stop() {
    if (server_fd != -1) close(server_fd);
}
