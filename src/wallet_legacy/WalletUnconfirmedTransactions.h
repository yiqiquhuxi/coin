// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "IWalletLegacy.h"
#include "ITransfersContainer.h"

#include <unordered_map>
#include <unordered_set>
#include <time.h>
#include <boost/functional/hash.hpp>

#include "cryptonote/core/key.h"
#include "crypto/crypto.h"

namespace cryptonote {
class ISerializer;

typedef std::pair<crypto::public_key_t, size_t> TransactionOutputId;
}

namespace std {

template<> 
struct hash<cryptonote::TransactionOutputId> {
  size_t operator()(const cryptonote::TransactionOutputId &_v) const {    
    return hash<crypto::public_key_t>()(_v.first) ^ _v.second;
  } 
}; 

}

namespace cryptonote {


struct UnconfirmedTransferDetails {

  UnconfirmedTransferDetails() :
    amount(0), sentTime(0), transactionId(WALLET_LEGACY_INVALID_TRANSACTION_ID) {}

  cryptonote::transaction_t tx;
  uint64_t amount;
  uint64_t outsAmount;
  time_t sentTime;
  TransactionId transactionId;
  std::vector<TransactionOutputId> usedOutputs;
};

class WalletUnconfirmedTransactions
{
public:

  explicit WalletUnconfirmedTransactions(uint64_t uncofirmedTransactionsLiveTime);

  bool serialize(cryptonote::ISerializer& s);

  bool findTransactionId(const crypto::hash_t& hash, TransactionId& id);
  void erase(const crypto::hash_t& hash);
  void add(const cryptonote::transaction_t& tx, TransactionId transactionId, 
    uint64_t amount, const std::list<TransactionOutputInformation>& usedOutputs);
  void updateTransactionId(const crypto::hash_t& hash, TransactionId id);

  uint64_t countUnconfirmedOutsAmount() const;
  uint64_t countUnconfirmedTransactionsAmount() const;
  bool isUsed(const TransactionOutputInformation& out) const;
  void reset();

  std::vector<TransactionId> deleteOutdatedTransactions();

private:

  void collectUsedOutputs();
  void deleteUsedOutputs(const std::vector<TransactionOutputId>& usedOutputs);

  typedef std::unordered_map<crypto::hash_t, UnconfirmedTransferDetails, boost::hash<crypto::hash_t>> UnconfirmedTxsContainer;
  typedef std::unordered_set<TransactionOutputId> UsedOutputsContainer;

  UnconfirmedTxsContainer m_unconfirmedTxs;
  UsedOutputsContainer m_usedOutputs;
  uint64_t m_uncofirmedTransactionsLiveTime;
};

} // namespace cryptonote
