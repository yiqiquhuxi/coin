// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <atomic>
#include <list>
#include <mutex>
#include <thread>

#include "cryptonote/core/key.h"
#include "cryptonote/core/currency.h"
#include "cryptonote/core/difficulty.h"
#include "cryptonote/core/IMinerHandler.h"
#include "command_line/MinerConfig.h"
#include "cryptonote/core/OnceInInterval.h"

#include <logging/LoggerRef.h>

#include "serialization/ISerializer.h"

namespace cryptonote {
  class miner {
  public:
    miner(const Currency& currency, IMinerHandler& handler, Logging::ILogger& log);
    ~miner();

    bool init(const MinerConfig& config);
    bool set_block_template(const block_t& bl, const difficulty_t& diffic);
    bool on_block_chain_update();
    bool start(const account_public_address_t& adr, size_t threads_count);
    uint64_t get_speed();
    void send_stop_signal();
    bool stop();
    bool is_mining();
    bool on_idle();
    void on_synchronized();
    //synchronous analog (for fast calls)
    static bool find_nonce_for_given_block(block_t& bl, const difficulty_t& diffic);
    void pause();
    void resume();
    void do_print_hashrate(bool do_hr);

  private:
    bool worker_thread(uint32_t th_local_index);
    bool request_block_template();
    void  merge_hr();

    struct miner_config
    {
      uint64_t current_extra_message_index;
      void serialize(ISerializer& s) {
        KV_MEMBER(current_extra_message_index)
      }
    };

    const Currency& m_currency;
    Logging::LoggerRef logger;

    std::atomic<bool> m_stop;
    std::mutex m_template_lock;
    block_t m_template;
    std::atomic<uint32_t> m_template_no;
    std::atomic<uint32_t> m_starter_nonce;
    difficulty_t m_diffic;

    std::atomic<uint32_t> m_threads_total;
    std::atomic<int32_t> m_pausers_count;
    std::mutex m_miners_count_lock;

    std::list<std::thread> m_threads;
    std::mutex m_threads_lock;
    IMinerHandler& m_handler;
    account_public_address_t m_mine_address;
    OnceInInterval m_update_block_template_interval;
    OnceInInterval m_update_merge_hr_interval;

    std::vector<binary_array_t> m_extra_messages;
    miner_config m_config;
    std::string m_config_folder_path;
    std::atomic<uint64_t> m_last_hr_merge_time;
    std::atomic<uint64_t> m_hashes;
    std::atomic<uint64_t> m_current_hash_rate;
    std::mutex m_last_hash_rates_lock;
    std::list<uint64_t> m_last_hash_rates;
    bool m_do_print_hashrate;
    bool m_do_mining;
  };
}
