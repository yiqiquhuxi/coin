#include "testnet.h"

namespace config
{
namespace testnet
{
const char GENESIS_COINBASE_TX_HEX[] = "013c01ff000101029b2e4c0281c0b02e7c53291a94d1d0cbff8883f8024f5142ee494ffbbd0880712101398fb9ec4e76aeef124dfb779de715022efd619e63d7516f8b1470266f5da1fd";
const char P2P_STAT_TRUSTED_PUB_KEY[] = "8f80f9a5a434a9f1510d13336228debfee9c918ce505efe225d8c94d045fa115";

const uint8_t BLOCK_MAJOR_VERSION_1 = 1;
const uint8_t BLOCK_MINOR_VERSION_0 = 0;
const uint8_t CURRENT_TRANSACTION_VERSION = 1;

namespace storage
{
const version_t blockcache_archive = {1, 0, 0};
const version_t blockcache_indices_archive = {1, 0, 0};
} // namespace storage

//TODO This port will be used by the daemon to establish connections with p2p network
const uint16_t P2P_DEFAULT_PORT = 29800;
//TODO This port will be used by the daemon to interact with simlewallet
const uint16_t RPC_DEFAULT_PORT = 29801;
//TODO This port will be used by the client wallets to interact with a service node
const uint16_t RPC_WALLET_PORT = 29888;

//TODO Put here the name of your currency
const char CRYPTONOTE_NAME[] = "vigcointest";

const uint64_t CRYPTONOTE_CREATION_TIME                      = 1520035200000;

seeds_t seeds = {
    "69.171.73.252:19800",
    "39.108.160.252:19800",
    "144.202.10.183:19800",
};

checkpoints_t checkpoints = {};
config_t data = {
    CRYPTONOTE_NAME,
    CRYPTONOTE_CREATION_TIME,
    // Block Info
    {GENESIS_COINBASE_TX_HEX,
     {BLOCK_MAJOR_VERSION_1, BLOCK_MINOR_VERSION_0, 0}},
    //Transaction Info
    {CURRENT_TRANSACTION_VERSION, 0, 0},
    {P2P_STAT_TRUSTED_PUB_KEY,
     P2P_DEFAULT_PORT,
     RPC_DEFAULT_PORT,
     RPC_WALLET_PORT,
     {1, 0, 0}},
    seeds,
    checkpoints};
}; // namespace testnet
} // namespace config