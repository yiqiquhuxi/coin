// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <list>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "IWalletLegacy.h"
#include "INode.h"
#include "Wallet/WalletErrors.h"
#include "Wallet/WalletAsyncContextCounter.h"
#include "Common/ObserverManager.h"
#include "CryptoNoteCore/TransactionExtra.h"
#include "CryptoNoteCore/CryptoNoteFormatUtils.h"
#include "CryptoNoteCore/Currency.h"
#include "WalletLegacy/WalletUserTransactionsCache.h"
#include "WalletLegacy/WalletUnconfirmedTransactions.h"

#include "WalletLegacy/WalletTransactionSender.h"
#include "WalletLegacy/WalletRequest.h"

#include "Transfers/BlockchainSynchronizer.h"
#include "Transfers/TransfersSynchronizer.h"

#include "Logging/LoggerGroup.h"

namespace CryptoNote
{
class WalletSingle : public IWalletLegacy,
                     IBlockchainSynchronizerObserver,
                     ITransfersObserver,
                     IWalletLegacyObserver
{

public:
  WalletSingle(const CryptoNote::Currency &currency, INode &node, Logging::LoggerGroup &logger);
  virtual ~WalletSingle();

  virtual void addObserver(IWalletLegacyObserver *observer) override;
  virtual void removeObserver(IWalletLegacyObserver *observer) override;

  virtual void initWithKeys(const AccountKeys &accountKeys, const std::string &password) override;

  virtual std::string getAddress() override;

  virtual uint64_t actualBalance() override;
  virtual uint64_t pendingBalance() override;

  virtual size_t getTransactionCount() override;
  virtual size_t getTransferCount() override;

  virtual TransactionId findTransactionByTransferId(TransferId transferId) override;

  virtual bool getTransaction(TransactionId transactionId, WalletLegacyTransaction &transaction) override;
  virtual bool getTransfer(TransferId transferId, WalletLegacyTransfer &transfer) override;

  virtual TransactionId sendTransaction(const WalletLegacyTransfer &transfer, uint64_t fee, const std::string &extra = "", uint64_t mixIn = 0, uint64_t unlockTimestamp = 0) override;
  virtual TransactionId sendTransaction(const std::vector<WalletLegacyTransfer> &transfers, uint64_t fee, const std::string &extra = "", uint64_t mixIn = 0, uint64_t unlockTimestamp = 0) override;
  virtual std::error_code cancelTransaction(size_t transactionId) override;

  virtual void getAccountKeys(AccountKeys &keys) override;

private:
  // IBlockchainSynchronizerObserver
  virtual void synchronizationProgressUpdated(uint32_t current, uint32_t total) override;
  virtual void synchronizationCompleted(std::error_code result) override;

  // ITransfersObserver
  virtual void onTransactionUpdated(ITransfersSubscription *object, const Crypto::Hash &transactionHash) override;
  virtual void onTransactionDeleted(ITransfersSubscription *object, const Crypto::Hash &transactionHash) override;

  void initSync();
  void throwIfNotInitialised();

  void synchronizationCallback(WalletRequest::Callback callback, std::error_code ec);
  void sendTransactionCallback(WalletRequest::Callback callback, std::error_code ec);
  void notifyClients(std::deque<std::shared_ptr<WalletLegacyEvent>> &events);
  void notifyIfBalanceChanged();

  std::vector<TransactionId> deleteOutdatedUnconfirmedTransactions();

  void initCompleted(std::error_code code);

  enum WalletState
  {
    NOT_INITIALIZED = 0,
    INITIALIZED,
    LOADING,
    SAVING
  };

  WalletState m_state;
  std::mutex m_cacheMutex;
  CryptoNote::AccountBase m_account;
  std::string m_password;
  const CryptoNote::Currency &m_currency;
  INode &m_node;
  bool m_isStopping;

  AccountKeys m_keys;

  std::atomic<uint64_t> m_lastNotifiedActualBalance;
  std::atomic<uint64_t> m_lastNotifiedPendingBalance;

  BlockchainSynchronizer m_blockchainSync;
  TransfersSyncronizer m_transfersSync;
  ITransfersContainer *m_transferDetails;

  WalletUserTransactionsCache m_transactionsCache;
  std::unique_ptr<WalletTransactionSender> m_sender;

  WalletAsyncContextCounter m_asyncContextCounter;
  Tools::ObserverManager<CryptoNote::IWalletLegacyObserver> m_observerManager;
  Logging::LoggerGroup &m_logger;
};

} //namespace CryptoNote