void KhutiWallet::startMiner(const std::string& pool_url, int pool_port) {
    // Build the command dynamically
    std::string cmd = "./xmrig -o " + pool_url + ":" + std::to_string(pool_port) +
                      " -u YOUR_WALLET_ADDRESS -p x -k --threads=4";

    // Debug: print the command
    std::cout << "[DEBUG] Launching: " << cmd << std::endl;

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        minerOutputQueue.push("🟡 Starting miner...");
        minerOutputQueue.push("⚡ Connecting to " + pool_url + ":" + std::to_string(pool_port));
        minerRunning = true;
    }
    queueCV.notify_all();

    FILE* fp = popen(cmd.c_str(), "r");
    if (!fp) {
        std::lock_guard<std::mutex> lock(queueMutex);
        minerOutputQueue.push("🔴 ERROR: Failed to launch miner. Is xmrig installed?");
        minerRunning = false;
        queueCV.notify_all();
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        std::string logLine = line;
        if (!logLine.empty() && logLine.back() == '\n') logLine.pop_back();
        if (!logLine.empty() && logLine.back() == '\r') logLine.pop_back();

        std::string coloredLine;
        if (logLine.find("sending") != std::string::npos) {
            coloredLine = "🟡 Sending: " + logLine;
        } else if (logLine.find("accepted") != std::string::npos) {
            coloredLine = "🟢 Approved: " + logLine;
            shares++;
        } else if (logLine.find("error") != std::string::npos || logLine.find("failed") != std::string::npos) {
            coloredLine = "🔴 Error: " + logLine;
        } else if (logLine.find("deposited") != std::string::npos || logLine.find("block") != std::string::npos) {
            coloredLine = "🔵 Deposited: " + logLine;
        } else {
            coloredLine = "⚪ " + logLine;
        }

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            minerOutputQueue.push(coloredLine);
        }
        queueCV.notify_all();

        std::cout << "[MINER] " << logLine << std::endl;
    }

    pclose(fp);
    minerRunning = false;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        minerOutputQueue.push("🔴 Miner stopped.");
    }
    queueCV.notify_all();
}
