#include <fstream>

#include <alibabacloud/credentials/Exception.hpp>
#include <darabonba/Core.hpp>

#include <alibabacloud/credentials/AuthUtil.hpp>
#include <alibabacloud/credentials/provider/OIDCRoleArnProvider.hpp>

namespace AlibabaCloud {
namespace Credentials {

OIDCRoleArnProvider::OIDCRoleArnProvider(std::shared_ptr<Models::Config> config,
                                         StaleValueBehavior behavior,
                                         std::shared_ptr<PrefetchStrategy> strategy)
    : RefreshableProvider(behavior, strategy),
      roleArn_(config->hasRoleArn() && !config->getRoleArn().empty()
                   ? config->getRoleArn()
                   : Darabonba::Env::getEnv(Constant::ENV_ROLE_ARN)),
      oidcProviderArn_(config->hasOidcProviderArn() && !config->getOidcProviderArn().empty()
                           ? config->getOidcProviderArn()
                           : Darabonba::Env::getEnv(Constant::ENV_OIDC_PROVIDER_ARN)),
      oidcTokenFilePath_(config->hasOidcTokenFilePath() && !config->getOidcTokenFilePath().empty()
                             ? config->getOidcTokenFilePath()
                             : Darabonba::Env::getEnv(Constant::ENV_OIDC_TOKEN_FILE)),
      roleSessionName_(config->hasRoleSessionName() && !config->getRoleSessionName().empty()
                           ? config->getRoleSessionName()
                           : (Darabonba::Env::getEnv(Constant::ENV_ROLE_SESSION_NAME).empty()
                                  ? "credentials-cpp-session"
                                  : Darabonba::Env::getEnv(Constant::ENV_ROLE_SESSION_NAME))),
      policy_(config->hasPolicy()
                  ? std::make_shared<std::string>(config->getPolicy())
                  : nullptr),
      durationSeconds_(config->getDurationSeconds()),
      regionId_(config->hasStsRegionId() && !config->getStsRegionId().empty()
                    ? config->getStsRegionId()
                    : (Darabonba::Env::getEnv(Constant::ENV_STS_REGION).empty()
                           ? config->getRegionId()
                           : Darabonba::Env::getEnv(Constant::ENV_STS_REGION))),
      stsEndpoint_(config->getStsEndpoint()),
      enableVpc_(config->hasEnableVpc()
                     ? config->getEnableVpc()
                     : (Darabonba::Env::getEnv(Constant::ENV_VPC_ENDPOINT_ENABLED) == "true")),
      connectTimeout_(config->hasConnectTimeout() ? config->getConnectTimeout() : 10000),
      readTimeout_(config->hasTimeout() ? config->getTimeout() : 5000) {
  if (roleArn_.empty()) {
    throw CredentialException(std::string("RoleArn cannot be empty."));
  }
  if (oidcProviderArn_.empty()) {
    throw CredentialException(std::string("OIDCProviderArn cannot be empty."));
  }
  if (oidcTokenFilePath_.empty()) {
    throw CredentialException(std::string("OIDCTokenFilePath cannot be empty."));
  }
}

OIDCRoleArnProvider::OIDCRoleArnProvider(const std::string &roleArn,
                                         const std::string &oidcProviderArn,
                                         const std::string &oidcTokenFilePath,
                                         const std::string &roleSessionName,
                                         std::shared_ptr<std::string> policy,
                                         int64_t durationSeconds,
                                         const std::string &regionId,
                                         const std::string &stsEndpoint,
                                         StaleValueBehavior behavior,
                                         std::shared_ptr<PrefetchStrategy> strategy)
    : RefreshableProvider(behavior, strategy),
      roleArn_(roleArn),
      oidcProviderArn_(oidcProviderArn),
      oidcTokenFilePath_(oidcTokenFilePath),
      roleSessionName_(roleSessionName.empty() ? "credentials-cpp-session" : roleSessionName),
      policy_(policy),
      durationSeconds_(durationSeconds > 0 ? durationSeconds : 3600),
      regionId_(regionId.empty() ? "cn-hangzhou" : regionId),
      stsEndpoint_(stsEndpoint.empty() ? "sts.aliyuncs.com" : stsEndpoint),
      enableVpc_(false),
      connectTimeout_(10000),
      readTimeout_(5000) {
  if (roleArn_.empty()) {
    throw CredentialException(std::string("RoleArn cannot be empty."));
  }
  if (oidcProviderArn_.empty()) {
    throw CredentialException(std::string("OIDCProviderArn cannot be empty."));
  }
  if (oidcTokenFilePath_.empty()) {
    throw CredentialException(std::string("OIDCTokenFilePath cannot be empty."));
  }
}

int64_t OIDCRoleArnProvider::getStaleTime(int64_t expiration) const {
  if (expiration <= 0) {
    return getCurrentTime() + 60 * 60;  // Default 1 hour if no expiration
  }
  return expiration - STALE_TIME_WINDOW;  // expiration - 15 minutes
}

RefreshResult OIDCRoleArnProvider::doRefresh() const {
  std::ifstream ifs(oidcTokenFilePath_);
  if (!ifs) {
    throw CredentialException(std::string("Can't open ") + oidcTokenFilePath_);
  }
  std::string oidcToken((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
  ifs.close();

  // V3 format: business parameters in Query
  Darabonba::Http::Query query = {
      {"DurationSeconds", std::to_string(durationSeconds_)},
      {"RoleArn", roleArn_},
      {"OIDCProviderArn", oidcProviderArn_},
      {"OIDCToken", oidcToken},
      {"RoleSessionName", roleSessionName_},
  };
  if (policy_) {
    query.emplace("Policy", *policy_);
  }

  // Build request
  std::string url = "https://" + stsEndpoint_ + "/";
  auto req = AuthUtil::getNewRequest(url);
  req.setQuery(std::move(query));
  req.setMethod("POST");

  // V3 format: common parameters in Header
  std::string utcDate = gmt_datetime();
  std::string nonce = Darabonba::Core::uuid();

  req.getHeaders()["host"] = stsEndpoint_;
  req.getHeaders()["x-acs-action"] = "AssumeRoleWithOIDC";
  req.getHeaders()["x-acs-version"] = "2015-04-01";
  req.getHeaders()["x-acs-date"] = utcDate;
  req.getHeaders()["x-acs-signature-nonce"] = nonce;
  // OIDC does not require signature, so no need to add Authorization Header

  // Use saved timeout configuration
  Darabonba::RuntimeOptions runtime;
  runtime.setConnectTimeout(connectTimeout_);
  runtime.setReadTimeout(readTimeout_);
  auto future = Darabonba::Core::doAction(req, runtime);
  auto resp = future.get();
  if (resp->getStatusCode() != 200) {
    throw CredentialException(Darabonba::Stream::readAsString(resp->getBody()));
  }
  auto result = Darabonba::Stream::readAsJSON(resp->getBody());
  auto &credential = result["Credentials"];
  int64_t expiration = strtotime(credential["Expiration"].get<std::string>());

  // Build credential model
  Models::CredentialModel cred;
  cred.setAccessKeyId(credential["AccessKeyId"].get<std::string>())
      .setAccessKeySecret(credential["AccessKeySecret"].get<std::string>())
      .setSecurityToken(credential["SecurityToken"].get<std::string>())
      .setType(Constant::OIDC_ROLE_ARN)
      .setProviderName(getProviderName());

  // Calculate stale time (expiration - 15 minutes)
  int64_t staleTime = getStaleTime(expiration);
  // Calculate prefetch time (stale time - 180 seconds)
  int64_t prefetchTime = staleTime - PREFETCH_THRESHOLD;

  return RefreshResult(cred, staleTime, prefetchTime);
}

} // namespace Credentials
} // namespace AlibabaCloud