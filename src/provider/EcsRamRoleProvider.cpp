#include <alibabacloud/credential/AuthUtil.hpp>
#include <alibabacloud/credential/provider/EcsRamRoleProvider.hpp>
#include <darabonba/Core.hpp>
#include <darabonba/Env.hpp>
#include <darabonba/encode/Encoder.hpp>
#include <memory>

namespace AlibabaCloud {
namespace Credential {

// C++11 requires out-of-class definition for constexpr static members
constexpr int EcsRamRoleProvider::DEFAULT_READ_TIMEOUT;
constexpr int EcsRamRoleProvider::DEFAULT_CONNECT_TIMEOUT;
constexpr int EcsRamRoleProvider::DEFAULT_METADATA_TOKEN_DURATION;

// 常量定义（对应 Python SDK）
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

// 构造函数实现
EcsRamRoleProvider::EcsRamRoleProvider(
    std::shared_ptr<Models::Config> config, bool asyncUpdateEnabled,
    StaleValueBehavior behavior, std::shared_ptr<PrefetchStrategy> strategy)
    : RefreshableProvider(behavior, strategy),
      roleName_(config->hasRoleName() ? config->getRoleName() : ""),
      disableIMDSv1_(config->hasDisableIMDSv1() ? config->getDisableIMDSv1()
                                                : false),
      shouldRefresh_(false), asyncUpdateEnabled_(asyncUpdateEnabled),
      connectTimeout_(config->hasConnectTimeout() ? config->getConnectTimeout()
                                                  : DEFAULT_CONNECT_TIMEOUT),
      readTimeout_(config->hasTimeout() ? config->getTimeout()
                                        : DEFAULT_READ_TIMEOUT) {

  // 检查是否禁用了 IMDS（对应 Python 的 environment_ecs_metadata_disabled
  // 检查）
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
}

// 获取 IMDSv2 Token（对应 Python 的 _get_metadata_token）
std::string EcsRamRoleProvider::getMetadataToken() const {
  std::string url =
      "http://" + META_DATA_SERVICE_HOST + URL_IN_ECS_METADATA_TOKEN;

  // 使用 getNewRequest 创建带 User-Agent 的请求（对应 Python SDK）
  auto req = AuthUtil::getNewRequest(url);
  req.setMethod("PUT");
  req.headers()["X-aliyun-ecs-metadata-token-ttl-seconds"] =
      std::to_string(DEFAULT_METADATA_TOKEN_DURATION);

  try {
    // 使用保存的超时配置
    Darabonba::RuntimeOptions runtime;
    runtime.setConnectTimeout(connectTimeout_);
    runtime.setReadTimeout(readTimeout_);
    auto future = Darabonba::Core::doAction(req, runtime);
    const auto resp = future.get();

    if (resp->statusCode() != 200) {
      throw Darabonba::Exception(
          ECS_METADATA_TOKEN_FETCH_ERROR_MSG +
          " HttpCode=" + std::to_string(resp->statusCode()));
    }

    return Darabonba::IFStream::readAsString(resp->body());
  } catch (const std::exception &e) {
    // 如果禁用了 IMDSv1，抛出异常
    if (disableIMDSv1_) {
      throw;
    }
    // 否则返回空，回退到 IMDSv1
    return "";
  }
}

// 刷新凭据（对应 Python 的 _refresh_credentials）
RefreshResult EcsRamRoleProvider::doRefresh() const {
  // 获取角色名（如果未设置）
  std::string roleNameToUse = roleName_;
  if (roleNameToUse.empty()) {
    roleNameToUse = getRoleName();
    roleName_ = roleNameToUse; // 缓存角色名
  }

  // 使用 getNewRequest 构建带 User-Agent 的请求（对应 Python SDK）
  std::string url =
      "http://" + META_DATA_SERVICE_HOST + URL_IN_ECS_META_DATA + roleNameToUse;
  auto req = AuthUtil::getNewRequest(url);

  // 尝试获取 IMDSv2 Token
  std::string metadataToken = getMetadataToken();
  if (!metadataToken.empty()) {
    req.headers()["X-aliyun-ecs-metadata-token"] = metadataToken;
  }

  // 发送请求，使用保存的超时配置
  Darabonba::RuntimeOptions runtime;
  runtime.setConnectTimeout(connectTimeout_);
  runtime.setReadTimeout(readTimeout_);
  auto future = Darabonba::Core::doAction(req, runtime);
  auto resp = future.get();

  if (resp->statusCode() != 200) {
    throw Darabonba::Exception(ECS_METADATA_FETCH_ERROR_MSG + " HttpCode=" +
                               std::to_string(resp->statusCode()));
  }

  // 解析响应
  auto result = Darabonba::IFStream::readAsJSON(resp->body());

  std::string contentCode = result["Code"].get<std::string>();
  if (contentCode != "Success") {
    throw Darabonba::Exception(ECS_METADATA_FETCH_ERROR_MSG +
                               " Code=" + contentCode);
  }

  // 提取凭据信息
  std::string accessKeyId = result["AccessKeyId"].get<std::string>();
  std::string accessKeySecret = result["AccessKeySecret"].get<std::string>();
  std::string securityToken = result["SecurityToken"].get<std::string>();
  std::string expirationStr = result["Expiration"].get<std::string>();

  // 解析过期时间（对应 Python 的 time.strptime 和 calendar.timegm）
  int64_t expiration = strtotime(expirationStr);

  // 计算 stale_time 和 prefetch_time（使用 Python 的逻辑）
  int64_t staleTime = getStaleTime(expiration);
  int64_t prefetchTime = getPrefetchTime(expiration);

  // 构建凭据
  Models::CredentialModel credential;
  credential.setType(Constant::ECS_RAM_ROLE)
      .setAccessKeyId(accessKeyId)
      .setAccessKeySecret(accessKeySecret)
      .setSecurityToken(securityToken)
      .setProviderName(getProviderName());

  // 设置 should_refresh 标志（对应 Python）
  shouldRefresh_ = true;

  // 返回刷新结果
  return RefreshResult(credential, staleTime, prefetchTime);
}

// 获取角色名（对应 Python 的 _get_role_name）
std::string EcsRamRoleProvider::getRoleName() const {
  std::string url = "http://" + META_DATA_SERVICE_HOST + URL_IN_ECS_META_DATA;

  // 使用 getNewRequest 创建带 User-Agent 的请求（对应 Python SDK）
  auto req = AuthUtil::getNewRequest(url);

  // 尝试获取 IMDSv2 Token
  std::string metadataToken = getMetadataToken();
  if (!metadataToken.empty()) {
    req.headers()["X-aliyun-ecs-metadata-token"] = metadataToken;
  }

  // 使用保存的超时配置
  Darabonba::RuntimeOptions runtime;
  runtime.setConnectTimeout(connectTimeout_);
  runtime.setReadTimeout(readTimeout_);
  auto future = Darabonba::Core::doAction(req, runtime);
  auto resp = future.get();

  if (resp->statusCode() != 200) {
    throw Darabonba::Exception(ECS_METADATA_FETCH_ERROR_MSG + " HttpCode=" +
                               std::to_string(resp->statusCode()));
  }

  return Darabonba::IFStream::readAsString(resp->body());
}

// 计算 stale_time（对应 Python 的 _get_stale_time）
int64_t EcsRamRoleProvider::getStaleTime(int64_t expiration) const {
  const int64_t now = getCurrentTime();

  // Python 逻辑：如果 expiration < 0，返回 now + 60分钟
  if (expiration < 0) {
    return now + 60 * 60;
  }

  // Python 逻辑：否则返回 expiration - 15分钟
  return expiration - 15 * 60;
}

// 计算 prefetch_time（对应 Python 的 _get_prefetch_time）
int64_t EcsRamRoleProvider::getPrefetchTime(int64_t expiration) {
  const int64_t now = getCurrentTime();

  // Python 逻辑：如果 expiration < 0，返回 now + 5分钟
  if (expiration < 0) {
    return now + 5 * 60;
  }

  // Python 逻辑：否则返回 now + 60分钟
  return now + 60 * 60;
}

} // namespace Credential
} // namespace AlibabaCloud
