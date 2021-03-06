// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "crypto/crypto.h"
#include "cryptonote/core/key.h"

#include "SingleTransactionTestBase.h"

class test_derive_public_key : public single_tx_test_base
{
public:
  static const size_t loop_count = 1000;

  bool init()
  {
    if (!single_tx_test_base::init())
      return false;

    crypto::generate_key_derivation(m_tx_pub_key, m_bob.getAccountKeys().viewSecretKey, m_key_derivation);
    m_spend_public_key = m_bob.getAccountKeys().address.spendPublicKey;

    return true;
  }

  bool test()
  {
    cryptonote::key_pair_t in_ephemeral;
    crypto::derive_public_key(m_key_derivation, 0, m_spend_public_key, in_ephemeral.publicKey);
    return true;
  }

private:
  crypto::key_derivation_t m_key_derivation;
  crypto::public_key_t m_spend_public_key;
};
