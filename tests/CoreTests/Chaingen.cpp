// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Chaingen.h"

#include <vector>
#include <iostream>
#include <stdint.h>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/program_options.hpp>

#include "command_line/options.h"
#include "cryptonote/core/key.h"
#include "crypto/crypto.h"
#include "crypto/hash.h"
#include "cryptonote/core/key.h"
#include "cryptonote/core/blockchain/serializer/crypto.h"
#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "cryptonote/core/core.h"
#include "cryptonote/core/currency.h"

//#include "AccountBoostSerialization.h"
//#include "cryptonote_boost_serialization.h"

using namespace std;
using namespace cryptonote;

struct output_index {
    const cryptonote::transaction_output_target_t out;
    uint64_t amount;
    size_t blk_height; // block height
    size_t tx_no; // index of transaction in block
    size_t out_no; // index of out in transaction
    uint32_t idx;
    bool spent;
    const cryptonote::block_t *p_blk;
    const cryptonote::transaction_t *p_tx;

    output_index(const cryptonote::transaction_output_target_t &_out, uint64_t _a, size_t _h, size_t tno, size_t ono, const cryptonote::block_t *_pb, const cryptonote::transaction_t *_pt)
        : out(_out), amount(_a), blk_height(_h), tx_no(tno), out_no(ono), idx(0), spent(false), p_blk(_pb), p_tx(_pt) { }

    output_index(const output_index &other)
        : out(other.out), amount(other.amount), blk_height(other.blk_height), tx_no(other.tx_no), out_no(other.out_no), idx(other.idx), spent(other.spent), p_blk(other.p_blk), p_tx(other.p_tx) {  }

    const std::string toString() const {
        std::stringstream ss;

        ss << "output_index{blk_height=" << blk_height
           << " tx_no=" << tx_no
           << " out_no=" << out_no
           << " amount=" << amount
           << " idx=" << idx
           << " spent=" << spent
           << "}";

        return ss.str();
    }

    output_index& operator=(const output_index& other)
    {
      new(this) output_index(other);
      return *this;
    }
};

typedef std::map<uint64_t, std::vector<size_t> > map_output_t;
typedef std::map<uint64_t, std::vector<output_index> > map_output_idx_t;
typedef pair<uint64_t, size_t>  outloc_t;

namespace
{
  uint64_t get_inputs_amount(const vector<transaction_source_entry_t> &s)
  {
    uint64_t r = 0;
    BOOST_FOREACH(const transaction_source_entry_t &e, s)
    {
      r += e.amount;
    }

    return r;
  }
}

bool init_output_indices(map_output_idx_t& outs, std::map<uint64_t, std::vector<size_t> >& outs_mine, const std::vector<cryptonote::block_t>& blockchain, const map_hash2tx_t& mtx, const cryptonote::Account& from) {

    BOOST_FOREACH (const block_t& blk, blockchain) {
        vector<const transaction_t*> vtx;
        vtx.push_back(&blk.baseTransaction);

        for (const crypto::hash_t& h : blk.transactionHashes) {
            const map_hash2tx_t::const_iterator cit = mtx.find(h);
            if (mtx.end() == cit)
                throw std::runtime_error("block contains an unknown tx hash");

            vtx.push_back(cit->second);
        }

        //vtx.insert(vtx.end(), blk.);
        // TODO: add all other txes
        for (size_t i = 0; i < vtx.size(); i++) {
            const transaction_t &tx = *vtx[i];

            size_t keyIndex = 0;
            for (size_t j = 0; j < tx.outputs.size(); ++j) {
              const transaction_output_t &out = tx.outputs[j];
              if (out.target.type() == typeid(key_output_t)) {
                output_index oi(out.target, out.amount, boost::get<base_input_t>(*blk.baseTransaction.inputs.begin()).blockIndex, i, j, &blk, vtx[i]);
                outs[out.amount].push_back(oi);
                uint32_t tx_global_idx = static_cast<uint32_t>(outs[out.amount].size() - 1);
                outs[out.amount][tx_global_idx].idx = tx_global_idx;
                // Is out to me?
                if (is_out_to_acc(from.getAccountKeys(), boost::get<key_output_t>(out.target), getTransactionPublicKeyFromExtra(tx.extra), keyIndex)) {
                  outs_mine[out.amount].push_back(tx_global_idx);
                }

                ++keyIndex;
              } else if (out.target.type() == typeid(multi_signature_output_t)) {
                keyIndex += boost::get<multi_signature_output_t>(out.target).keys.size();
              }
            }
        }
    }

    return true;
}

bool init_spent_output_indices(map_output_idx_t& outs, map_output_t& outs_mine, const std::vector<cryptonote::block_t>& blockchain, const map_hash2tx_t& mtx, const cryptonote::Account& from) {

    for (const map_output_t::value_type& o: outs_mine) {
        for (size_t i = 0; i < o.second.size(); ++i) {
            output_index &oi = outs[o.first][o.second[i]];

            // construct key image for this output
            crypto::key_image_t img;
            key_pair_t in_ephemeral;
            generate_key_image_helper(from.getAccountKeys(), getTransactionPublicKeyFromExtra(oi.p_tx->extra), oi.out_no, in_ephemeral, img);

            // lookup for this key image in the events vector
            for (auto& tx_pair : mtx) {
                const transaction_t& tx = *tx_pair.second;
                for (const auto& in : tx.inputs) {
                    if (in.type() == typeid(key_input_t)) {
                        const key_input_t &itk = boost::get<key_input_t>(in);
                        if (itk.keyImage == img) {
                            oi.spent = true;
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool fill_output_entries(std::vector<output_index>& out_indices, size_t sender_out, size_t nmix, size_t& real_entry_idx, std::vector<transaction_source_entry_t::output_entry_t>& output_entries)
{
  if (out_indices.size() <= nmix)
    return false;

  bool sender_out_found = false;
  size_t rest = nmix;
  for (size_t i = 0; i < out_indices.size() && (0 < rest || !sender_out_found); ++i)
  {
    const output_index& oi = out_indices[i];
    if (oi.spent)
      continue;

    bool append = false;
    if (i == sender_out)
    {
      append = true;
      sender_out_found = true;
      real_entry_idx = output_entries.size();
    }
    else if (0 < rest)
    {
      --rest;
      append = true;
    }

    if (append)
    {
      const key_output_t& otk = boost::get<key_output_t>(oi.out);
      output_entries.push_back(transaction_source_entry_t::output_entry_t(oi.idx, otk.key));
    }
  }

  return 0 == rest && sender_out_found;
}

bool fill_tx_sources(std::vector<transaction_source_entry_t>& sources, const std::vector<test_event_entry>& events,
                     const block_t& blk_head, const cryptonote::Account& from, uint64_t amount, size_t nmix)
{
    map_output_idx_t outs;
    map_output_t outs_mine;

    std::vector<cryptonote::block_t> blockchain;
    map_hash2tx_t mtx;
    if (!find_block_chain(events, blockchain, mtx, Block::getHash(blk_head)))
        return false;

    if (!init_output_indices(outs, outs_mine, blockchain, mtx, from))
        return false;

    if (!init_spent_output_indices(outs, outs_mine, blockchain, mtx, from))
        return false;

    // Iterate in reverse is more efficiency
    uint64_t sources_amount = 0;
    bool sources_found = false;
    BOOST_REVERSE_FOREACH(const map_output_t::value_type o, outs_mine)
    {
        for (size_t i = 0; i < o.second.size() && !sources_found; ++i)
        {
            size_t sender_out = o.second[i];
            const output_index& oi = outs[o.first][sender_out];
            if (oi.spent)
                continue;

            cryptonote::transaction_source_entry_t ts;
            ts.amount = oi.amount;
            ts.realOutputIndexInTransaction = oi.out_no;
            ts.realTransactionPublicKey = getTransactionPublicKeyFromExtra(oi.p_tx->extra); // incoming tx public key
            size_t realOutput;
            if (!fill_output_entries(outs[o.first], sender_out, nmix, realOutput, ts.outputs))
              continue;

            ts.realOutput = realOutput;

            sources.push_back(ts);

            sources_amount += ts.amount;
            sources_found = amount <= sources_amount;
        }

        if (sources_found)
            break;
    }

    return sources_found;
}

bool fill_tx_destination(transaction_destination_entry_t &de, const cryptonote::Account &to, uint64_t amount) {
    de.addr = to.getAccountKeys().address;
    de.amount = amount;
    return true;
}

void fill_tx_sources_and_destinations(const std::vector<test_event_entry>& events, const block_t& blk_head,
                                      const cryptonote::Account& from, const cryptonote::Account& to,
                                      uint64_t amount, uint64_t fee, size_t nmix, std::vector<transaction_source_entry_t>& sources,
                                      std::vector<transaction_destination_entry_t>& destinations)
{
  sources.clear();
  destinations.clear();

  if (!fill_tx_sources(sources, events, blk_head, from, amount + fee, nmix))
    throw std::runtime_error("couldn't fill transaction sources");

  transaction_destination_entry_t de;
  if (!fill_tx_destination(de, to, amount))
    throw std::runtime_error("couldn't fill transaction destination");
  destinations.push_back(de);

  transaction_destination_entry_t de_change;
  uint64_t cache_back = get_inputs_amount(sources) - (amount + fee);
  if (0 < cache_back)
  {
    if (!fill_tx_destination(de_change, from, cache_back))
      throw std::runtime_error("couldn't fill transaction cache back destination");
    destinations.push_back(de_change);
  }
}

bool construct_tx_to_key(Logging::ILogger& logger, const std::vector<test_event_entry>& events, cryptonote::transaction_t& tx, const block_t& blk_head,
                         const cryptonote::Account& from, const cryptonote::Account& to, uint64_t amount,
                         uint64_t fee, size_t nmix)
{
  vector<transaction_source_entry_t> sources;
  vector<transaction_destination_entry_t> destinations;
  fill_tx_sources_and_destinations(events, blk_head, from, to, amount, fee, nmix, sources, destinations);

  return constructTransaction(from.getAccountKeys(), sources, destinations, std::vector<uint8_t>(), tx, 0, logger);
}

transaction_t construct_tx_with_fee(Logging::ILogger& logger, std::vector<test_event_entry>& events, const block_t& blk_head,
                                  const Account& acc_from, const Account& acc_to, uint64_t amount, uint64_t fee)
{
  transaction_t tx;
  construct_tx_to_key(logger, events, tx, blk_head, acc_from, acc_to, amount, fee, 0);
  events.push_back(tx);
  return tx;
}

uint64_t get_balance(const cryptonote::Account& addr, const std::vector<cryptonote::block_t>& blockchain, const map_hash2tx_t& mtx) {
    uint64_t res = 0;
    std::map<uint64_t, std::vector<output_index> > outs;
    std::map<uint64_t, std::vector<size_t> > outs_mine;

    map_hash2tx_t confirmed_txs;
    get_confirmed_txs(blockchain, mtx, confirmed_txs);

    if (!init_output_indices(outs, outs_mine, blockchain, confirmed_txs, addr))
        return false;

    if (!init_spent_output_indices(outs, outs_mine, blockchain, confirmed_txs, addr))
        return false;

    BOOST_FOREACH (const map_output_t::value_type &o, outs_mine) {
        for (size_t i = 0; i < o.second.size(); ++i) {
            if (outs[o.first][o.second[i]].spent)
                continue;

            res += outs[o.first][o.second[i]].amount;
        }
    }

    return res;
}

void get_confirmed_txs(const std::vector<cryptonote::block_t>& blockchain, const map_hash2tx_t& mtx, map_hash2tx_t& confirmed_txs)
{
  std::unordered_set<crypto::hash_t> confirmed_hashes;
  for (const block_t& blk : blockchain)
  {
    for (const crypto::hash_t& tx_hash : blk.transactionHashes)
    {
      confirmed_hashes.insert(tx_hash);
    }
  }

  BOOST_FOREACH(const auto& tx_pair, mtx)
  {
    if (0 != confirmed_hashes.count(tx_pair.first))
    {
      confirmed_txs.insert(tx_pair);
    }
  }
}

bool find_block_chain(const std::vector<test_event_entry>& events, std::vector<cryptonote::block_t>& blockchain, map_hash2tx_t& mtx, const crypto::hash_t& head) {
    std::unordered_map<crypto::hash_t, const block_t*> block_index;
    BOOST_FOREACH(const test_event_entry& ev, events)
    {
        if (typeid(block_t) == ev.type())
        {
            const block_t* blk = &boost::get<block_t>(ev);
            block_index[Block::getHash(*blk)] = blk;
        }
        else if (typeid(transaction_t) == ev.type())
        {
            const transaction_t& tx = boost::get<transaction_t>(ev);
            mtx[BinaryArray::objectHash(tx)] = &tx;
        }
    }

    bool b_success = false;
    crypto::hash_t id = head;
    for (auto it = block_index.find(id); block_index.end() != it; it = block_index.find(id))
    {
        blockchain.push_back(*it->second);
        id = it->second->previousBlockHash;
        if (NULL_HASH == id)
        {
            b_success = true;
            break;
        }
    }
    reverse(blockchain.begin(), blockchain.end());

    return b_success;
}


const cryptonote::Currency& test_chain_unit_base::currency() const
{
  return m_currency;
}

void test_chain_unit_base::register_callback(const std::string& cb_name, verify_callback cb)
{
  m_callbacks[cb_name] = cb;
}

bool test_chain_unit_base::verify(const std::string& cb_name, cryptonote::core& c, size_t ev_index, const std::vector<test_event_entry> &events)
{
  auto cb_it = m_callbacks.find(cb_name);
  if(cb_it == m_callbacks.end())
  {
    LOG_ERROR("Failed to find callback " << cb_name);
    return false;
  }
  return cb_it->second(c, ev_index, events);
}
