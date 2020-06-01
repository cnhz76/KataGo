#ifdef BUILD_DISTRIBUTED

#ifndef DISTRIBUTED_CLIENT_H_
#define DISTRIBUTED_CLIENT_H_

#include "../core/logger.h"
#include "../distributed/httplib_wrapper.h"
#include "../core/multithread.h"
#include "../dataio/trainingwrite.h"

namespace Client {

  struct RunParameters {
    std::string runName;
    std::string infoUrl;
    int dataBoardLen;
    int inputsVersion;
    int maxSearchThreadsAllowed;
  };

  struct ModelInfo {
    std::string name;
    std::string infoUrl;
    std::string downloadUrl;
    int64_t bytes;
    std::string sha256;
    bool isRandom;
  };

  struct Task {
    std::string taskId;
    std::string taskGroup;
    std::string runName;
    std::string runInfoUrl;

    ModelInfo modelBlack;
    ModelInfo modelWhite;

    std::string config;
    bool doWriteTrainingData;
    bool isRatingGame;
  };

  class Connection {
  public:
    Connection(const std::string& serverUrl, const std::string& username, const std::string& password, Logger* logger);
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) = delete;
    Connection& operator=(Connection&&) = delete;

    RunParameters getRunParameters();
    //Returns true if a task was obtained. Returns false if no task was obtained, but not due to an error (e.g. shouldStop).
    //Raises an exception upon a repeated error that persists long enough.
    bool getNextTask(
      Task& task,
      const std::string& baseDir,
      bool retryOnFailure, std::atomic<bool>& shouldStop
    );

    static std::string getModelPath(const Client::ModelInfo& modelInfo, const std::string& modelDir);

    //Returns true if a model was downloaded or download was not necessary.
    //Returns false if a model count not be downloaded, but not due to an error (e.g. shouldStop).
    //Raises an exception upon a repeated error that persists long enough.
    bool downloadModelIfNotPresent(
      const Client::ModelInfo& modelInfo, const std::string& modelDir,
      bool retryOnFailure, std::atomic<bool>& shouldStop
    );

    //Returns true if data was uploaded or upload was not needed.
    //Returns false if it was not, but not due to an error (e.g. shouldStop).
    //Raises an exception upon a repeated error that persists long enough.
    bool uploadTrainingGameAndData(
      const Task& task, const FinishedGameData* gameData, const std::string& sgfFilePath, const std::string& npzFilePath,
      bool retryOnFailure, std::atomic<bool>& shouldStop
    );
    bool uploadRatingGame(
      const Task& task, const FinishedGameData* gameData, const std::string& sgfFilePath,
      bool retryOnFailure, std::atomic<bool>& shouldStop
    );

  private:
    std::shared_ptr<httplib::Response> get(const std::string& subPath);
    std::shared_ptr<httplib::Response> post(const std::string& subPath, const std::string& data, const std::string& dtype);
    std::shared_ptr<httplib::Response> postMulti(const std::string& subPath, const httplib::MultipartFormDataItems& data);


    httplib::Client* httpClient;
    httplib::SSLClient* httpsClient;
    bool isSSL;
    std::string baseResourcePath;

    Logger* logger;

    //TODO if httplib is thread-safe, then we can remove this
    std::mutex mutex;
  };

}

#endif //DISTRIBUTED_CLIENT_H_

#endif //BUILD_DISTRIBUTED
