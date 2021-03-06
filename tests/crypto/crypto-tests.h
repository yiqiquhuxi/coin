// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#if defined(__cplusplus)
#include "crypto/crypto.h"

extern "C" {
#endif

void setup_random(void);

#if defined(__cplusplus)
}

bool check_scalar(const crypto::elliptic_curve_scalar_t &scalar);
void random_scalar(crypto::elliptic_curve_scalar_t &res);
void hash_to_scalar(const void *data, size_t length, crypto::elliptic_curve_scalar_t &res);
void hash_to_point(const crypto::hash_t &h, crypto::elliptic_curve_point_t &res);
void hash_to_ec(const crypto::public_key_t &key, crypto::elliptic_curve_point_t &res);
#endif
