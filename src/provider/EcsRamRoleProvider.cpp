#include <alibabacloud/credentials/AuthUtil.hpp>
#include <alibabacloud/credentials/Exception.hpp>
#include <alibabacloud/credentials/provider/EcsRamRoleProvider.hpp>
#include <alibabacloud/credentials/provider/EcsScheduler.hpp>
#include <darabonba/Core.hpp>
#include <darabonba/Env.hpp>
#include <darabonba/encode/Encoder.hpp>
#include <memory>

namespace AlibabaCloud {
namespace Credentials {

// Note: In C++17+, inline variables make class-external definitions redundant.
// However, for C++11/14 compatibility, we need these definitions when the constant
// is ODR-used (e.g., as a function parameter or with std::to_string).
#if __cplusplus < 201703L
constexpr int EcsRamRoleProvider::DEFAULT_METADATA_TOKEN_DURATION;
constexpr int EcsRamRoleProvider::DEFAULT_CONNECT_TIMEOUT;
constexpr int EcsRamRoleProvider::DEFAULT_READ_TIMEOUT;
#endif

// 常量定义
const std::string EcsRamRoleProvider::URL_IN_ECS_META_DATA =
    "/latest/meta-data/ram/security-credentials/";
const std::string EcsRamRoleProvider::URL_IN_ECS_METADATA_TOKEN =
    "/latest/api/token";
const std::string EcsRamRoleProvider::META_DATA_SERVICE_HOST =
    "100.100.100.200";
const std::string EcsRamRoleProvider::ECS_METADATA_FETCH_ERROR_MSG =
    "Failed to get RAM session credentials from ECS metadata service.";
const std::string EcsRamRoleProvider::ECS_METADATA_TOKEN_FETCH_ERROR_MSG =
    "Failed to get token from ECS Metadata Service.";

// 析构函数
EcsRamRoleProvider::~EcsRamRoleProvider() {
  unregisterFromScheduler();
}

// 注册到全局调度器
void EcsRamRoleProvider::registerWithScheduler() {
  if (!asyncUpdateEnabled_) {
    return;
  }

  // 注册刷新回调到全局调度器
  schedulerEntryId_ = EcsScheduler::instance().registerCallback([this]() {
    scheduledRefresh();
  });
}

// 从全局调度器注销
void EcsRamRoleProvider::unregisterFromScheduler() {
  if (schedulerEntryId_ != 0) {
    EcsScheduler::instance().unregister(schedulerEntryId_);
    schedulerEntryId_ = 0;
  }
}

// 定时刷新回调（由全局调度器调用）
void EcsRamRoleProvider::scheduledRefresh() {
  if (shouldRefresh_.load()) {
    try {
      getCredential();
    } catch (const std::exception&) {
      // 忽略刷新错误，下次继续尝试
    }
  }
}

// 构造函数实现
EcsRamRoleProvider::EcsRamRoleProvider(
    std::shared_ptr<Models::Config> config, bool asyncUpdateEnabled,
    StaleValueBehavior behavior, std::shared_ptr<PrefetchStrategy> strategy)
    : RefreshableProvider(behavior, strategy),
      roleName_(config->hasRoleName() ? config->getRoleName() : ""),
      disableIMDSv1_(config->hasDisableIMDSv1() ? config->getDisableIMDSv1()
                                                : false),
      shouldRefresh_(false),
      asyncUpdateEnabled_(asyncUpdateEnabled),
      connectTimeout_(config->hasConnectTimeout() ? config->getConnectTimeout()
                                                  : DEFAULT_CONNECT_TIMEOUT),
      readTimeout_(config->hasTimeout() ? config->getTimeout()
                                        : DEFAULT_READ_TIMEOUT) {

  // 检查是否禁用了 IMDS
  std::string ecsMetadataDisabled =
      Darabonba::Env::getEnv("ALIBABA_CLOUD_ECS_METADATA_DISABLED");
  if (!ecsMetadataDisabled.empty() &&
      (ecsMetadataDisabled == "true" || ecsMetadataDisabled == "TRUE")) {
    throw std::runtime_error("IMDS credentials is disabled");
  }

  // 如果未设置角色名，尝试从环境变量获取
  if (roleName_.empty()) {
    roleName_ = Darabonba::Env::getEnv("ALIBABA_CLOUD_ECS_METADATA");
  }

  // 如果未设置 disableIMDSv1，检查环境变量
  if (!disableIMDSv1_) {
    std::string imdsv1Disabled =
        Darabonba::Env::getEnv("ALIBABA_CLOUD_IMDSV1_DISABLED");
    disableIMDSv1_ = (!imdsv1Disabled.empty() &&
                      (imdsv1Disabled == "true" || imdsv1Disabled == "TRUE"));
  }

  // 注册到全局调度器
  registerWithScheduler();
}

EcsRamRoleProvider::EcsRamRoleProvider(
    const std::string &roleName, bool disableIMDSv1, bool asyncUpdateEnabled,
    StaleValueBehavior behavior, std::shared_ptr<PrefetchStrategy> strategy)
    : RefreshableProvider(behavior, strategy), roleName_(roleName),
      disableIMDSv1_(disableIMDSv1), shouldRefresh_(false),
      asyncUpdateEnabled_(asyncUpdateEnabled),
      connectTimeout_(DEFAULT_CONNECT_TIMEOUT),
      readTimeout_(DEFAULT_READ_TIMEOUT) {

  // 检查环境变量
  std::string ecsMetadataDisabled =
      Darabonba::Env::getEnv("ALIBABA_CLOUD_ECS_METADATA_DISABLED");
  if (!ecsMetadataDisabled.empty() &&
      (ecsMetadataDisabled == "true" || ecsMetadataDisabled == "TRUE")) {
    throw std::runtime_error("IMDS credentials is disabled");
  }

  if (roleName_.empty()) {
    roleName_ = Darabonba::Env::getEnv("ALIBABA_CLOUD_ECS_METADATA");
  }

  if (!disableIMDSv1_) {
    std::string imdsv1Disabled =
        Darabonba::Env::getEnv("ALIBABA_CLOUD_IMDSV1_DISABLED");
    disableIMDSv1_ = (!imdsv1Disabled.empty() &&
                      (imdsv1Disabled == "true" || imdsv1Disabled == "TRUE"));
  }

  // 注册到全局调度器
  registerWithScheduler();
}

// 获取 IMDSv2 Token
std::string EcsRamRoleProvider::getMetadataToken() const {
  std::string url =
      "http://" + META_DATA_SERVICE_HOST + URL_IN_ECS_METADATA_TOKEN;

  auto req = AuthUtil::getNewRequest(url);
  req.setMethod("PUT");
  req.getHeaders()["X-aliyun-ecs-metadata-token-ttl-seconds"] =
      std::to_string(DEFAULT_METADATA_TOKEN_DURATION);

  try {
    Darabonba::RuntimeOptions runtime;
    runtime.setConnectTimeout(connectTimeout_);
    runtime.setReadTimeout(readTimeout_);
    auto future = Darabonba::Core::doAction(req, runtime);
    const auto resp = future.get();

    if (resp->getStatusCode() != 200) {
      throw CredentialException(
          ECS_METADATA_TOKEN_FETCH_ERROR_MSG +
          " HttpCode=" + std::to_string(resp->getStatusCode()));
    }

    return Darabonba::IFStream::readAsString(resp->getBody());
  } catch (const std::exception&) {
    if (disableIMDSv1_) {
      throw;
    }
    return "";
  }
}

// 刷新凭据
RefreshResult EcsRamRoleProvider::doRefresh() const {
  std::string roleNameToUse = roleName_;
  if (roleNameToUse.empty()) {
    roleNameToUse = getRoleName();
    roleName_ = roleNameToUse;
  }

  std::string url =
      "http://" + META_DATA_SERVICE_HOST + URL_IN_ECS_META_DATA + roleNameToUse;
  auto req = AuthUtil::getNewRequest(url);

  std::string metadataToken = getMetadataToken();
  if (!metadataToken.empty()) {
    req.getHeaders()["X-aliyun-ecs-metadata-token"] = metadataToken;
  }

  Darabonba::RuntimeOptions runtime;
  runtime.setConnectTimeout(connectTimeout_);
  runtime.setReadTimeout(readTimeout_);

  std::shared_ptr<Darabonba::Http::MCurlResponse> resp;
  try {
    auto future = Darabonba::Core::doAction(req, runtime);
    resp = future.get();
  } catch (const std::exception& e) {
    throw CredentialException(
        ECS_METADATA_FETCH_ERROR_MSG +
        " This may indicate you are not running in an ECS/ECI environment. "
        "Error: " + std::string(e.what()));
  }

  if (!resp) {
    throw CredentialException(
        ECS_METADATA_FETCH_ERROR_MSG +
        " No response received. Please ensure you are running in an ECS/ECI environment.");
  }

  if (resp->getStatusCode() == 0) {
    throw CredentialException(
        ECS_METADATA_FETCH_ERROR_MSG +
        " Connection failed (HttpCode=0). This usually means you are not running in an ECS/ECI environment. "
        "The ECS metadata service (100.100.100.200) is only accessible from within Alibaba Cloud ECS instances.");
  }

  if (resp->getStatusCode() != 200) {
    throw CredentialException(ECS_METADATA_FETCH_ERROR_MSG + " HttpCode=" +
                               std::to_string(resp->getStatusCode()));
  }

  auto result = Darabonba::IFStream::readAsJSON(resp->getBody());

  std::string contentCode = result["Code"].get<std::string>();
  if (contentCode != "Success") {
    throw CredentialException(ECS_METADATA_FETCH_ERROR_MSG +
                               " Code=" + contentCode);
  }

  std::string accessKeyId = result["AccessKeyId"].get<std::string>();
  std::string accessKeySecret = result["AccessKeySecret"].get<std::string>();
  std::string securityToken = result["SecurityToken"].get<std::string>();
  std::string expirationStr = result["Expiration"].get<std::string>();

  int64_t expiration = strtotime(expirationStr);
  int64_t staleTime = getStaleTime(expiration);
  int64_t prefetchTime = getPrefetchTime(expiration);

  Models::CredentialModel credential;
  credential.setType(Constant::ECS_RAM_ROLE)
      .setAccessKeyId(accessKeyId)
      .setAccessKeySecret(accessKeySecret)
      .setSecurityToken(securityToken)
      .setProviderName(getProviderName());

  shouldRefresh_ = true;

  return RefreshResult(credential, staleTime, prefetchTime);
}

// 获取角色名
std::string EcsRamRoleProvider::getRoleName() const {
  std::string url = "http://" + META_DATA_SERVICE_HOST + URL_IN_ECS_META_DATA;

  auto req = AuthUtil::getNewRequest(url);

  std::string metadataToken = getMetadataToken();
  if (!metadataToken.empty()) {
    req.getHeaders()["X-aliyun-ecs-metadata-token"] = metadataToken;
  }

  Darabonba::RuntimeOptions runtime;
  runtime.setConnectTimeout(connectTimeout_);
  runtime.setReadTimeout(readTimeout_);

  std::shared_ptr<Darabonba::Http::MCurlResponse> resp;
  try {
    auto future = Darabonba::Core::doAction(req, runtime);
    resp = future.get();
  } catch (const std::exception& e) {
    throw CredentialException(
        ECS_METADATA_FETCH_ERROR_MSG +
        " Failed to get role name. This may indicate you are not running in an ECS/ECI environment. "
        "Error: " + std::string(e.what()));
  }

  if (!resp || resp->getStatusCode() == 0) {
    throw CredentialException(
        ECS_METADATA_FETCH_ERROR_MSG +
        " Connection failed. Please ensure you are running in an ECS/ECI environment.");
  }

  if (resp->getStatusCode() != 200) {
    throw CredentialException(ECS_METADATA_FETCH_ERROR_MSG + " HttpCode=" +
                               std::to_string(resp->getStatusCode()));
  }

  return Darabonba::IFStream::readAsString(resp->getBody());
}

// 计算 stale_time
int64_t EcsRamRoleProvider::getStaleTime(int64_t expiration) const {
  const int64_t now = getCurrentTime();

  if (expiration < 0) {
    return now + 60 * 60;
  }

  return expiration - 15 * 60;
}

// 计算 prefetch_time
int64_t EcsRamRoleProvider::getPrefetchTime(int64_t expiration) {
  const int64_t now = getCurrentTime();

  if (expiration < 0) {
    return now + 5 * 60;
  }

  return now + 60 * 60;
}

} // namespace Credentials
} // namespace AlibabaCloud