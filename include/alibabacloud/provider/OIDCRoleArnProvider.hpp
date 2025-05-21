#ifndef AlibabaCloud_CREDENTIAL_OIDCROLEARNPROVIDER_HPP_
#define AlibabaCloud_CREDENTIAL_OIDCROLEARNPROVIDER_HPP_

#include <alibabacloud/Constant.hpp>
// #include <alibabacloud/provider/NeedFreshProvider.hpp>
#include <alibabacloud/provider/Provider.hpp>

namespace AlibabaCloud {
namespace Credential {

class OIDCRoleArnProvider {
public:
  OIDCRoleArnProvider(std::shared_ptr<Models::Config> config)
      : roleArn_(config->roleArn()),
        oidcProviderArn_(config->oidcProviderArn()),
        oidcTokenFilePath_(config->oidcTokenFilePath()),
        roleSessionName_(config->roleSessionName()),
        policy_(config->hasPolicy()
                    ? std::make_shared<std::string>(config->policy())
                    : nullptr),
        durationSeconds_(config->durationSeconds()),
        regionId_(config->regionId()), stsEndpoint_(config->stsEndpoint()) {
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

protected:
  // virtual bool refreshCredential() const override;

  mutable Models::Credential credential_;

  std::string roleArn_;
  std::string oidcProviderArn_;
  std::string oidcTokenFilePath_;
  std::string roleSessionName_;
  std::shared_ptr<std::string> policy_ = nullptr;
  int64_t durationSeconds_ = 3600;
  std::string regionId_ = "cn-hangzhou";
  std::string stsEndpoint_ = "sts.aliyuncs.com";
};
} // namespace Credential
} // namespace AlibabaCloud

#endif