#pragma once

#include <Swoosh/Activity.h>
#include <Swoosh/Ease.h>
#include <Swoosh/Timer.h>
#include <Swoosh/EmbedGLSL.h>
#include <Swoosh/Shaders.h>

#include <time.h>
#include <typeinfo>
#include <SFML/Graphics.hpp>
#include <Poco/Net/DatagramSocket.h>
#include <Poco/Buffer.h>

#include "bnNetPlayPacketProcessor.h"
#include "../bnScene.h"
#include "../bnText.h"
#include "../../bnInputManager.h"
#include "../../bnDrawWindow.h"
#include "../../bnSceneNode.h"
#include "../../bnWebClientMananger.h"

struct DownloadSceneProps {
  bool& downloadSuccess;
  std::vector<std::string> cardUUIDs;
  Poco::Net::SocketAddress remoteAddress;
  std::shared_ptr<Netplay::PacketProcessor> packetProcessor;
  sf::Texture& lastScreen;
};

class DownloadScene final : public Scene {
private:
  enum class state : char {
    trade = 0,
    download,
    complete
  } currState{};

  bool& downloadSuccess;
  bool aborting{}, remoteSuccess{};
  frame_time_t abortingCountdown{frames(150)};
  size_t tries{}; //!< After so many attempts, quit the download...
  size_t packetAckId{};
  std::vector<std::string> ourCardList, retryCardList;
  std::map<std::string, std::string> cardsToDownload;
  Text label;
  sf::Sprite bg; // background
  sf::RenderTexture surface;
  sf::Texture lastScreen;
  std::shared_ptr<Netplay::PacketProcessor> packetProcessor;
  swoosh::glsl::FastGaussianBlur blur{ 10 };

  void TradeCardList(const std::vector<std::string>& uuids);
  void RequestCardList(const std::vector<std::string>& uuids);

  void SendCustomPlayerData();
  void SendDownloadComplete(bool success);
  void SendPing(); //!< keep connections alive while clients download data

  void RecieveTradeCardList(const Poco::Buffer<char>& buffer);
  void RecieveRequestCardList(const Poco::Buffer<char>& buffer);
  void RecieveDownloadComplete(const Poco::Buffer<char>& buffer);
  void DownloadCardList(const Poco::Buffer<char>& buffer);
 
  std::vector<std::string> DeserializeUUIDs(const Poco::Buffer<char>& buffer);
  Poco::Buffer<char> SerializeUUIDs(NetPlaySignals header, const std::vector<std::string>& uuids);
  Poco::Buffer<char> SerializeCards(const std::vector<std::string>& cardList);
  void Abort(const std::vector<std::string>& failed);
  void ProcessPacketBody(NetPlaySignals header, const Poco::Buffer<char>& body);

public:
  void onUpdate(double elapsed) override final;
  void onDraw(sf::RenderTexture& surface) override final;
  void onLeave() override;
  void onExit() override;
  void onEnter() override;
  void onStart() override;
  void onResume() override;
  void onEnd() override;

  /**
   * @brief Construct scene from previous screen's contents
   */
  DownloadScene(swoosh::ActivityController&, const DownloadSceneProps& props);
  ~DownloadScene();

};