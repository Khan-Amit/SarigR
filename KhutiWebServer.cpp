else if (path == "/start_rig" && method == "POST") {
    if (!minerRunning) {
        try {
            nlohmann::json req = nlohmann::json::parse(body);
            std::string pool_url = req.value("pool_url", "xmr-ae.kryptex.network");
            int pool_port = req.value("pool_port", 7777);
            std::thread([this, pool_url, pool_port]() {
                wallet.startMiner(pool_url, pool_port);
            }).detach();
            response_body = R"({"status":"miner_started"})";
        } catch (...) {
            // fallback to default
            std::thread([this]() { wallet.startMiner(); }).detach();
            response_body = R"({"status":"miner_started"})";
        }
    } else {
        response_body = R"({"status":"already_running"})";
    }
    content_type = "application/json";
}
