/*! \file bnQueueCardRegistration.h */
/*! \brief This function hooks into the loading phase and loads extra content */

#pragma once

#include "bindings/bnScriptedCard.h"

 // TODO: mac os < 10.5 file system support...
#ifndef __APPLE__
#include <filesystem>
#endif 

static inline void QueueCardRegistration(CardPackageManager& packageManager) {
  // ResourceHandle handle;

#if defined(BN_MOD_SUPPORT) && !defined(__APPLE__)
  // Script resource manager load scripts from designated folder "resources/mods/cards"
  std::string path_str = "resources/mods/cards";
  for (const auto& entry : std::filesystem::directory_iterator(path_str)) {
    auto full_path = std::filesystem::absolute(entry).string();

    if (full_path.find(".zip") == std::string::npos) {
      // TODO: check paths first to get rid of try-catches
      try {
        if (auto res = packageManager.LoadPackageFromDisk<ScriptedCard>(full_path); res.is_error()) {
          Logger::Logf("%s", res.error_cstr());
        }
      }
      catch (std::runtime_error& e) {
        Logger::Logf("Card package unknown error: %s", e.what());
      }
    }
  }
#endif
}