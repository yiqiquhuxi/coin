#pragma once

#include "block.h"
#include "array.h"
#include "cryptonote/core/blockchain/serializer/transaction_entry.hpp"
#include "cryptonote/core/key.h"

namespace cryptonote
{
struct block_info_t
{
    uint32_t height;
    crypto::hash_t id;

    block_info_t()
    {
        clear();
    }

    void clear()
    {
        height = 0;
        id = cryptonote::NULL_HASH;
    }

    bool empty() const
    {
        return id == cryptonote::NULL_HASH;
    }
};
struct block_entry_t
{
    block_t bl;
    uint32_t height;
    uint64_t block_cumulative_size;
    difficulty_t cumulative_difficulty;
    uint64_t already_generated_coins;
    std::vector<transaction_entry_t> transactions;

    void serialize(ISerializer &s)
    {
        s(bl, "block");
        s(height, "height");
        s(block_cumulative_size, "block_cumulative_size");
        s(cumulative_difficulty, "cumulative_difficulty");
        s(already_generated_coins, "already_generated_coins");
        s(transactions, "transactions");
    }
};

class Block
{
  public:
    Block(block_entry_t &block) : m_block(block) {};

    //Static tool functions
    static bool getBlob(const block_t &b, binary_array_t &ba);
    static bool getHash(const block_t &block, crypto::hash_t &hash);
    static crypto::hash_t getHash(const block_t &block);

    static bool getLongHash(const block_t &b, crypto::hash_t &res);
    static bool checkProofOfWork(const block_t &block, difficulty_t currentDiffic, crypto::hash_t &proofOfWork);

    static block_t genesis(config::config_t &conf);

    static std::string toString(const block_entry_t &block);

    std::string toString();

  private:
    block_entry_t m_block;
};
} // namespace cryptonote