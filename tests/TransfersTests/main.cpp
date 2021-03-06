// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"
#include "Globals.h"

#include <logging/ConsoleLogger.h>

Logging::ConsoleLogger logger;
System::Dispatcher globalSystem;
cryptonote::Currency currency = cryptonote::CurrencyBuilder(os::appdata::path(), config::testnet::data, logger)
// .testnet(true)
.currency();
Tests::Common::BaseFunctionalTestsConfig testConfig;


namespace po = boost::program_options;

int main(int argc, char** argv) {
  CLogger::Instance().init(CLogger::DEBUG);

  po::options_description desc;
  po::variables_map vm;
  
  testConfig.init(desc);
  po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);
  po::notify(vm);
  testConfig.handleCommandLine(vm);

  try {

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

  } catch (std::exception& ex) {
    LOG_ERROR("Fatal error: " + std::string(ex.what()));
    return 1;
  }
}
