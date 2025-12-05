#ifndef ALIBABACLOUD_CREDENTIAL_RAMROLEARNPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_RAMROLEARNPROVIDER_HPP_

#include <string>

#include <darabonba/Env.hpp>

#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/provider/NeedFreshProvider.hpp>

namespace AlibabaCloud {
namespace Credential {

class RamRoleArnProvider : public NeedFreshProvider,
                           std::enable_shared_from_this<RamRoleArnProvider> {
public:
  RamRoleArnProvider(std::shared_ptr<Models::Config> config)
      : roleArn_(config->roleArn()),
        roleSessionName_(config->roleSessionName()),
        policy_(config->hasPolicy()
                    ? std::make_shared<std::string>(config->policy())
                    : nullptr),
        durationSeconds_(config->durationSeconds()), 
        regionId_(config->hasStsRegionId() && !config->stsRegionId().empty() 
                      ? config->stsRegionId()
                      : (Darabonba::Env::getEnv(Constant::ENV_STS_REGION).empty()
                             ? config->regionId()
                             : Darabonba::Env::getEnv(Constant::ENV_STS_REGION))),
        stsEndpoint_(config->stsEndpoint()),
        enableVpc_(config->hasEnableVpc()
                       ? config->enableVpc()
                       : (Darabonba::Env::getEnv(Constant::ENV_VPC_ENDPOINT_ENABLED) == "true")),
        connectTimeout_(config->hasConnectTimeout() ? config->connectTimeout() : 10000),
        readTimeout_(config->hasTimeout() ? config->timeout() : 5000) {
    credential_.setAccessKeyId(config->accessKeyId())
        .setAccessKeySecret(config->accessKeySecret())
        .setType(Constant::RAM_ROLE_ARN);
  }

  RamRoleArnProvider(const std::string &accessKeyId,
                     const std::string &accessKeySecret,
                     const std::string &roleArn,
                     const std::string roleSessionName,
                     std::shared_ptr<std::string> policy = nullptr,
                     int64_t durationSeconds_ = 3600,
                     const std::string &regionId = "cn-hangzhou",
                     const std::string &stsEndpoint = "sts.aliyuncs.com")
      : roleArn_(roleArn), roleSessionName_(roleSessionName), policy_(policy),
        durationSeconds_(durationSeconds_), regionId_(regionId),
        stsEndpoint_(stsEndpoint) {
    credential_.setAccessKeyId(accessKeyId)
        .setAccessKeySecret(accessKeySecret)
        .setType(Constant::RAM_ROLE_ARN);
  }

  virtual ~RamRoleArnProvider() {}
  
  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override { return Constant::RAM_ROLE_ARN; }

private:
  virtual bool refreshCredential() const override;

  mutable Models::CredentialModel credential_;
  std::string roleArn_;
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