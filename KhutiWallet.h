#ifndef KHUTIWALLET_H
#define KHUTIWALLET_H

#include <string>
#include <vector>
#include <cstdint>

class KhutiWallet {
private:
    std::string wallet_path;
    std::string address;
    unsigned char private_key[32];
    std::string public_key_hex;

    double onchain_balance;
    double hawala_pending_balance;
    uint64_t last_block_height;
    double hashrate_khs;
    int shares;

    bool is_loaded;

    void generate_keypair();
    std::string pubkey_to_address(const std::vector<unsigned char>& pubkey_hash);
    std::vector<unsigned char> base58_decode(const std::string& str);
    std::string base58_encode(const std::vector<unsigned char>& data);

public:
    KhutiWallet();
    bool create_new(const std::string& password, const std::string& save_path);
    bool load_from_file(const std::string& password, const std::string& path);
    bool save_to_file(const std::string& password);

    std::string getAddress() const { return address; }
    double getOnChainBalance() const { return onchain_balance; }
    double getHawalaPending() const { return hawala_pending_balance; }
    uint64_t getLastBlockHeight() const { return last_block_height; }
    double getSweeperHashRate() const { return hashrate_khs; }
    int getShares() const { return shares; }

    std::string createHawalaToken(const std::string& receiver_addr, double amount, uint64_t nonce);
    bool acceptHawalaToken(const std::string& base64_token);

    void sweep_vintage_utxos();
    void startMiner();   // launches xmrig
    void update_balance_from_node(NetworkClient& client);
};

#endif
