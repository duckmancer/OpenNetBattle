#include "bnWebClientMananger.h"
#include "bnXPlatformStringCopy.h"
#include "bnURLParser.h"
#include "bnLogger.h"
#include "bnElements.h"

#include <SFML/Network/Http.hpp>

void WebClientManager::PingThreadHandler()
{
    do {
        std::unique_lock<std::mutex> lock(this->clientMutex);

        if (!this->client) {
            this->isConnected = false;
        }
        else {
            this->isConnected = this->client->IsOK();
        }      

        lock.unlock();

        std::this_thread::sleep_for(std::chrono::microseconds(this->GetPingInterval()));
    } while (!shutdownSignal);
}

void WebClientManager::QueuedTasksThreadHandler()
{
    std::unique_lock<std::mutex> lock(this->clientMutex);

    do {
        //Wait until we have data
        taskQueueWakeup.wait(lock, [this] {
            return (taskQueue.size());
        });

        //after wait, we own the lock
        if (taskQueue.size())
        {
            this->isWorking = true;
            auto op = std::move(taskQueue.front());
            taskQueue.pop();

            lock.unlock();
            op();
            lock.lock();
            this->isWorking = false;
        }
    } while (!shutdownSignal);
}

void WebClientManager::InitDownloadImageHandler()
{
    if (!this->client) return;

    auto callback = [](const char* url, WebAccounts::byte*& image) -> void {
        sf::Http Http;
        sf::Http::Request request;
        size_t size = 0;

        URL urlParser(url);

        Http.setHost(urlParser.GetHost());
        request.setMethod(sf::Http::Request::Get);
        request.setUri(urlParser.GetPath());

        std::cout << urlParser.GetHost() << ", " << urlParser.GetPath() << ", " << urlParser.GetQuery() << std::endl;

        sf::Http::Response Page = Http.sendRequest(request);

        size = Page.getBody().size();

        std::string data = Page.getBody();

        image = new WebAccounts::byte[size + 1];
        if (size > 0) {
            XPLATFORM_STRCPY((char*)image, size + 1, data.data());
            image[size] = '\0';
        }
    };

    std::scoped_lock<std::mutex>(this->clientMutex);
    this->client->SetDownloadImageHandler(callback);
}

void WebClientManager::CacheTextureData(const WebAccounts::AccountState& account)
{
    /*
    for (auto&& card : account.cards) {
        auto cardModel = account.cardModels.find(card.second.modelId);

        auto imageData = cardModel->second.imageData;
        auto iconData = cardModel->second.iconData;

        sf::Texture textureObject;
        textureObject.loadFromMemory(imageData, strlen((char*)imageData));
        this->cardTextureCache.insert(std::make_pair(card.first, textureObject));

        textureObject = sf::Texture();
        textureObject.loadFromMemory(iconData, strlen((char*)iconData));
        this->iconTextureCache.insert(std::make_pair(card.first, textureObject));
    }*/
}

WebClientManager::WebClientManager() {
    shutdownSignal = false;
    isConnected = false;
    isWorking = false;

    PingInterval(2000);

    pingThread = std::thread(&WebClientManager::PingThreadHandler, this);
    pingThread.detach();

    tasksThread = std::thread(&WebClientManager::QueuedTasksThreadHandler, this);
    tasksThread.detach();
}

WebClientManager& WebClientManager::GetInstance() {
    static WebClientManager instance;
    return instance;
}

void WebClientManager::PingInterval(long interval) {
    this->heartbeatInterval = interval;
}

const long WebClientManager::GetPingInterval() const {
    return this->heartbeatInterval;
}

void WebClientManager::ConnectToWebServer(const char * apiVersion, const char * domain, int port)
{
    this->client = std::make_unique<WebAccounts::WebClient>(apiVersion, domain, port);
    InitDownloadImageHandler();
}

const bool WebClientManager::IsConnectedToWebServer()
{
    return isConnected;
}

const bool WebClientManager::IsLoggedIn()
{
    return this->client ? this->client->IsLoggedIn() : false;
}

const bool WebClientManager::IsWorking()
{
    return true || this->isWorking;
}

std::future<bool> WebClientManager::SendLoginCommand(const char * username, const char * password)
{
    auto promise = std::make_shared<std::promise<bool>>();

    auto task = [promise, username, password, this]() {
        if (!this->client) {
            // No valid client? Set to false immediately
            promise->set_value(false);
            return;
        }

        bool result = this->client->Login(username, password);
        promise->set_value(result);
    };

    std::scoped_lock<std::mutex>(this->clientMutex);

    this->taskQueue.emplace(task);

    this->taskQueueWakeup.notify_all();

    return promise->get_future();
}

std::future<bool> WebClientManager::SendLogoutCommand()
{
    auto promise = std::make_shared<std::promise<bool>>();

    auto task = [promise, this]() {
        if (!this->client) {
            // No valid client? Set to false immediately
            promise->set_value(false);
            return;
        }

        this->client->LogoutAndReset();

        // We should be logged out
        promise->set_value(!this->client->IsLoggedIn());
    };

    std::scoped_lock<std::mutex>(this->clientMutex);

    this->taskQueue.emplace(task);

    this->taskQueueWakeup.notify_all();

    return promise->get_future();
}

std::future<WebAccounts::AccountState> WebClientManager::SendFetchAccountCommand()
{
    auto promise = std::make_shared<std::promise<WebAccounts::AccountState>>();

    auto task = [promise, this]() {
        if (!this->client) {
            // No valid client? Don't send invalid data. Throw.
            promise->set_exception(std::make_exception_ptr(std::runtime_error("Could not get account data. Client object is invalid.")));
            return;
        }

        this->client->FetchAccount();
        promise->set_value(this->client->GetLocalAccount());
    };

    std::scoped_lock<std::mutex>(this->clientMutex);

    this->taskQueue.emplace(task);

    this->taskQueueWakeup.notify_all();

    return promise->get_future();
}

const sf::Texture& WebClientManager::GetIconForCard(const std::string & uuid)
{
    return iconTextureCache[uuid];
}

const sf::Texture& WebClientManager::GetImageForCard(const std::string & uuid)
{
    return cardTextureCache[uuid];
}

Card WebClientManager::MakeBattleCardFromWebCardData(const WebAccounts::AccountState& account, const WebAccounts::Card & card)
{
    auto modelIter = account.cardModels.find(card.modelId);
    auto cardModel = modelIter->second;

    return Card(card.id, card.code, cardModel.damage, GetElementFromStr(cardModel.element),
        cardModel.name, cardModel.description, cardModel.verboseDescription, 0);
}

void WebClientManager::ShutdownAllTasks()
{
    shutdownSignal = true;

    std::unique_lock<std::mutex> lock(clientMutex);
    while (taskQueue.size()) {
        taskQueue.pop();
    }
    lock.unlock();

    this->taskQueueWakeup.notify_all();

    if (tasksThread.joinable()) {
        tasksThread.join();
    }

    if (pingThread.joinable()) {
        pingThread.join();
    }
}
