// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstddef>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "crypto/crypto.h"
#include "crypto/hash.h"
#include "crypto-tests.h"
#include "../io.h"

using namespace std;
typedef crypto::hash_t chash;

bool operator !=(const crypto::elliptic_curve_scalar_t &a, const crypto::elliptic_curve_scalar_t &b) {
  return 0 != memcmp(&a, &b, sizeof(crypto::elliptic_curve_scalar_t));
}

bool operator !=(const crypto::elliptic_curve_point_t &a, const crypto::elliptic_curve_point_t &b) {
  return 0 != memcmp(&a, &b, sizeof(crypto::elliptic_curve_point_t));
}

bool operator !=(const crypto::key_derivation_t &a, const crypto::key_derivation_t &b) {
  return 0 != memcmp(&a, &b, sizeof(crypto::key_derivation_t));
}

int main(int argc, char *argv[]) {
  fstream input;
  string cmd;
  size_t test = 0;
  bool error = false;
  setup_random();
  if (argc != 2) {
    cerr << "invalid arguments" << endl;
    return 1;
  }
  input.open(argv[1], ios_base::in);
  for (;;) {
    ++test;
    input.exceptions(ios_base::badbit);
    if (!(input >> cmd)) {
      break;
    }
    input.exceptions(ios_base::badbit | ios_base::failbit | ios_base::eofbit);
    if (cmd == "check_scalar") {
      crypto::elliptic_curve_scalar_t scalar;
      bool expected, actual;
      get(input, scalar, expected);
      actual = check_scalar(scalar);
      if (expected != actual) {
        goto error;
      }
    } else if (cmd == "random_scalar") {
      crypto::elliptic_curve_scalar_t expected, actual;
      get(input, expected);
      random_scalar(actual);
      if (expected != actual) {
        goto error;
      }
    } else if (cmd == "hash_to_scalar") {
      vector<char> data;
      crypto::elliptic_curve_scalar_t expected, actual;
      get(input, data, expected);
      hash_to_scalar(data.data(), data.size(), actual);
      if (expected != actual) {
        goto error;
      }
    } else if (cmd == "generate_keys") {
      crypto::public_key_t expected1, actual1;
      crypto::secret_key_t expected2, actual2;
      get(input, expected1, expected2);
      generate_keys(actual1, actual2);
      if (expected1 != actual1 || expected2 != actual2) {
        goto error;
      }
    } else if (cmd == "check_key") {
      crypto::public_key_t key;
      bool expected, actual;
      get(input, key, expected);
      actual = check_key(key);
      if (expected != actual) {
        goto error;
      }
    } else if (cmd == "secret_key_to_public_key") {
      crypto::secret_key_t sec;
      bool expected1, actual1;
      crypto::public_key_t expected2, actual2;
      get(input, sec, expected1);
      if (expected1) {
        get(input, expected2);
      }
      actual1 = secret_key_to_public_key(sec, actual2);
      if (expected1 != actual1 || (expected1 && expected2 != actual2)) {
        goto error;
      }
    } else if (cmd == "generate_key_derivation") {
      crypto::public_key_t key1;
      crypto::secret_key_t key2;
      bool expected1, actual1;
      crypto::key_derivation_t expected2, actual2;
      get(input, key1, key2, expected1);
      if (expected1) {
        get(input, expected2);
      }
      actual1 = generate_key_derivation(key1, key2, actual2);
      if (expected1 != actual1 || (expected1 && expected2 != actual2)) {
        goto error;
      }
    } else if (cmd == "derive_public_key") {
      crypto::key_derivation_t derivation;
      size_t output_index;
      crypto::public_key_t base;
      bool expected1, actual1;
      crypto::public_key_t expected2, actual2;
      get(input, derivation, output_index, base, expected1);
      if (expected1) {
        get(input, expected2);
      }
      actual1 = derive_public_key(derivation, output_index, base, actual2);
      if (expected1 != actual1 || (expected1 && expected2 != actual2)) {
        goto error;
      }
    } else if (cmd == "derive_secret_key") {
      crypto::key_derivation_t derivation;
      size_t output_index;
      crypto::secret_key_t base;
      crypto::secret_key_t expected, actual;
      get(input, derivation, output_index, base, expected);
      derive_secret_key(derivation, output_index, base, actual);
      if (expected != actual) {
        goto error;
      }
    } else if (cmd == "underive_public_key") {
      crypto::key_derivation_t derivation;
      size_t output_index;
      crypto::public_key_t derived_key;
      bool expected1, actual1;
      crypto::public_key_t expected2, actual2;
      get(input, derivation, output_index, derived_key, expected1);
      if (expected1) {
        get(input, expected2);
      }
      actual1 = underive_public_key(derivation, output_index, derived_key, actual2);
      if (expected1 != actual1 || (expected1 && expected2 != actual2)) {
        goto error;
      }
    } else if (cmd == "generate_signature") {
      chash prefix_hash;
      crypto::public_key_t pub;
      crypto::secret_key_t sec;
      crypto::signature_t expected, actual;
      get(input, prefix_hash, pub, sec, expected);
      generate_signature(prefix_hash, pub, sec, actual);
      if (expected != actual) {
        goto error;
      }
    } else if (cmd == "check_signature") {
      chash prefix_hash;
      crypto::public_key_t pub;
      crypto::signature_t sig;
      bool expected, actual;
      get(input, prefix_hash, pub, sig, expected);
      actual = check_signature(prefix_hash, pub, sig);
      if (expected != actual) {
        goto error;
      }
    } else if (cmd == "hash_to_point") {
      chash h;
      crypto::elliptic_curve_point_t expected, actual;
      get(input, h, expected);
      hash_to_point(h, actual);
      if (expected != actual) {
        goto error;
      }
    } else if (cmd == "hash_to_ec") {
      crypto::public_key_t key;
      crypto::elliptic_curve_point_t expected, actual;
      get(input, key, expected);
      hash_to_ec(key, actual);
      if (expected != actual) {
        goto error;
      }
    } else if (cmd == "generate_key_image") {
      crypto::public_key_t pub;
      crypto::secret_key_t sec;
      crypto::key_image_t expected, actual;
      get(input, pub, sec, expected);
      generate_key_image(pub, sec, actual);
      if (expected != actual) {
        goto error;
      }
    } else if (cmd == "generate_ring_signature") {
      chash prefix_hash;
      crypto::key_image_t image;
      vector<crypto::public_key_t> vpubs;
      vector<const crypto::public_key_t *> pubs;
      size_t pubs_count;
      crypto::secret_key_t sec;
      size_t sec_index;
      vector<crypto::signature_t> expected, actual;
      size_t i;
      get(input, prefix_hash, image, pubs_count);
      vpubs.resize(pubs_count);
      pubs.resize(pubs_count);
      for (i = 0; i < pubs_count; i++) {
        get(input, vpubs[i]);
        pubs[i] = &vpubs[i];
      }
      get(input, sec, sec_index);
      expected.resize(pubs_count);
      getvar(input, pubs_count * sizeof(crypto::signature_t), expected.data());
      actual.resize(pubs_count);
      generate_ring_signature(prefix_hash, image, pubs.data(), pubs_count, sec, sec_index, actual.data());
      if (expected != actual) {
        goto error;
      }
    } else if (cmd == "check_ring_signature") {
      chash prefix_hash;
      crypto::key_image_t image;
      vector<crypto::public_key_t> vpubs;
      vector<const crypto::public_key_t *> pubs;
      size_t pubs_count;
      vector<crypto::signature_t> sigs;
      bool expected, actual;
      size_t i;
      get(input, prefix_hash, image, pubs_count);
      vpubs.resize(pubs_count);
      pubs.resize(pubs_count);
      for (i = 0; i < pubs_count; i++) {
        get(input, vpubs[i]);
        pubs[i] = &vpubs[i];
      }
      sigs.resize(pubs_count);
      getvar(input, pubs_count * sizeof(crypto::signature_t), sigs.data());
      get(input, expected);
      actual = check_ring_signature(prefix_hash, image, pubs.data(), pubs_count, sigs.data());
      if (expected != actual) {
        goto error;
      }
    } else {
      throw ios_base::failure("Unknown function: " + cmd);
    }
    continue;
error:
    cerr << "Wrong result on test " << test << endl;
    error = true;
  }
  return error ? 1 : 0;
}
