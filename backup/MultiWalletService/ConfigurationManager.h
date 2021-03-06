// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "command_line/CoreConfig.h"
#include "GateConfiguration.h"
#include "command_line/NetNodeConfig.h"
#include "RpcNodeConfiguration.h"

namespace MultiWalletService {

class ConfigurationManager {
public:
  ConfigurationManager();
  bool init(int argc, char** argv);

  bool startInprocess;
  Configuration gateConfiguration;
  cryptonote::NetNodeConfig netNodeConfig;
  cryptonote::CoreConfig coreConfig;
  RpcNodeConfiguration remoteNodeConfig;
};

} //namespace MultiWalletService
