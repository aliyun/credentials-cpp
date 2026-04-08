#include <darabonba/Core.hpp>

#include <alibabacloud/credentials/AuthUtil.hpp>
#include <alibabacloud/credentials/Exception.hpp>
#include <alibabacloud/credentials/provider/URLProvider.hpp>

namespace AlibabaCloud {
namespace Credentials {

URLProvider::URLProvider(std::shared_ptr<Models::Config> config,
                         StaleValueBehavior behavior,
                         std::shared_ptr<PrefetchStrategy> strategy)
    : RefreshableProvider(behavior, strategy),
      url_(config->getCredentialsUri()),
      connectTimeout_(config->hasConnectTimeout() ? config->getConnectTimeout() : 10000),
      readTimeout_(config->hasTimeout() ? config->getTimeout() : 5000) {
  if (url_.empty()) {
    throw CredentialException(std::string("URL cannot be empty."));
  }
}

URLProvider::URLProvider(const std::string &url,
                         StaleValueBehavior behavior,
                         std::shared_ptr<PrefetchStrategy> strategy)
    : RefreshableProvider(behavior, strategy),
      url_(url) {
  if (url.empty()) {
    throw CredentialException(std::string("URL cannot be empty."));
  }
}

int64_t URLProvider::getStaleTime(int64_t expiration) const {
  if (expiration <= 0) {
    return getCurrentTime() + 60 * 60;  // Default 1 hour if no expiration
  }
  return expiration - STALE_TIME_WINDOW;  // expiration - 15 minutes
}

RefreshResult URLProvider::doRefresh() const {
  // 使用 getNewRequest 创建带 User-Agent 的请求（对应 Python SDK）
  auto req = AuthUtil::getNewRequest(url_);
  // Use saved timeout configuration
  Darabonba::RuntimeOptions runtime;
  runtime.setConnectTimeout(connectTimeout_);
  runtime.setReadTimeout(readTimeout_);
  auto future = Darabonba::Core::doAction(req, runtime);
  auto resp = future.get();

  // Check for network failure (resp is nullptr or status code 0)
  if (!resp) {
    throw CredentialException("Failed to connect to credentials URL: " + url_);
  }

  int statusCode = static_cast<int>(resp->getStatusCode());
  if (statusCode != 200) {
    std::string body = Darabonba::Stream::readAsString(resp->getBody());
    std::string errorMsg = body.empty()
        ? "HTTP request failed with status code " + std::to_string(statusCode)
        : body;
    throw CredentialException(errorMsg);
  }
  const auto &result = Darabonba::Stream::readAsJSON(resp->getBody());
  if (result["Code"].get<std::string>() != "Success") {
    throw CredentialException(result.dump());
  }
  int64_t expiration = strtotime(result["Expiration"].get<std::string>());

  // Build credential model
  Models::CredentialModel cred;
  cred.setAccessKeyId(result["AccessKeyId"].get<std::string>())
      .setAccessKeySecret(result["AccessKeySecret"].get<std::string>())
      .setSecurityToken(result["SecurityToken"].get<std::string>())
      .setType(Constant::URL_STS)
      .setProviderName(getProviderName());

  // Calculate stale time (expiration - 15 minutes)
  int64_t staleTime = getStaleTime(expiration);
  // Calculate prefetch time (stale time - 180 seconds)
  int64_t prefetchTime = staleTime - PREFETCH_THRESHOLD;

  return RefreshResult(cred, staleTime, prefetchTime);
}

} // namespace Credentials
} // namespace AlibabaCloud