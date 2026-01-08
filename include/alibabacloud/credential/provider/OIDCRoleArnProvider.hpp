#ifndef ALIBABACLOUD_CREDENTIAL_OIDCROLEARNPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_OIDCROLEARNPROVIDER_HPP_

#include <darabonba/Env.hpp>

#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/provider/NeedFreshProvider.hpp>
#include <alibabacloud/credential/provider/Provider.hpp>

namespace AlibabaCloud {
namespace Credential {

class OIDCRoleArnProvider : public NeedFreshProvider,
                           std::enable_shared_from_this<OIDCRoleArnProvider>{
public:
  OIDCRoleArnProvider(std::shared_ptr<Models::Config> config)
      : roleArn_(config->hasRoleArn() && !config->getRoleArn().empty()
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
                                    ? "defaultSessionName"
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
    credential_.setType(Constant::OIDC_ROLE_ARN);
  }

  OIDCRoleArnProvider(const std::string &roleArn,
                      const std::string &oidcProviderArn,
                      const std::string &oidcTokenFilePath,
                      const std::string &roleSessionName = "defaultSessionName",
                      std::shared_ptr<std::string> policy = nullptr,
                      int64_t durationSeconds = 3600,
                      const std::string &regionId = "cn-hangzhou",
                      const std::string &stsEndpoint = "sts.aliyuncs.com")
      : roleArn_(roleArn), oidcProviderArn_(oidcProviderArn),
        oidcTokenFilePath_(oidcTokenFilePath),
        roleSessionName_(roleSessionName), policy_(policy),
        durationSeconds_(durationSeconds), regionId_(regionId),
        stsEndpoint_(stsEndpoint) {
    credential_.setType(Constant::OIDC_ROLE_ARN);
  }
  virtual ~OIDCRoleArnProvider() = default;
  
  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override { return Constant::OIDC_ROLE_ARN; }

protected:
  virtual bool refreshCredential() const override;
  mutable Models::CredentialModel credential_;

  std::string roleArn_;
  std::string oidcProviderArn_;
  std::string oidcTokenFilePath_;
  std::string roleSessionName_;
  std::shared_ptr<std::string> policy_ = nullptr;
  int64_t durationSeconds_ = 3600;
  std::string regionId_ = "cn-hangzhou";
  std::string stsEndpoint_ = "sts.aliyuncs.com";
  bool enableVpc_ = false;
  int64_t connectTimeout_ = 10000;  // Connection timeout in milliseconds
  int64_t readTimeout_ = 5000;      // Read timeout in milliseconds
};
} // namespace Credential
} // namespace AlibabaCloud

#endif