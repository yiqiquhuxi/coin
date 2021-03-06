// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "crypto/crypto.cpp"

#include "crypto-tests.h"

bool check_scalar(const crypto::elliptic_curve_scalar_t &scalar) {
  return crypto::sc_check(reinterpret_cast<const unsigned char*>(&scalar)) == 0;
}

void random_scalar(crypto::elliptic_curve_scalar_t &res) {
  crypto::random_scalar(res);
}

void hash_to_scalar(const void *data, size_t length, crypto::elliptic_curve_scalar_t &res) {
  crypto::hash_to_scalar(data, length, res);
}

void hash_to_point(const crypto::hash_t &h, crypto::elliptic_curve_point_t &res) {
  crypto::ge_p2 point;
  crypto::ge_fromfe_frombytes_vartime(&point, reinterpret_cast<const unsigned char *>(&h));
  crypto::ge_tobytes(reinterpret_cast<unsigned char*>(&res), &point);
}

void hash_to_ec(const crypto::public_key_t &key, crypto::elliptic_curve_point_t &res) {
  crypto::ge_p3 tmp;
  crypto::hash_to_ec(key, tmp);
  crypto::ge_p3_tobytes(reinterpret_cast<unsigned char*>(&res), &tmp);
}
