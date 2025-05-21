#ifndef ALIBABACLOUD_CREDENTIAL_RAMROLEARNPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_RAMROLEARNPROVIDER_HPP_
#include <alibabacloud/credential/Config.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/provider/NeedFreshProvider.hpp>
#include <string>

namespace Alibabacloud {
namespace Credential {

class RamRoleArnProvider : public NeedFreshProvider,
                           std::enable_shared_from_this<RamRoleArnProvider> {
public:
  RamRoleArnProvider(std::shared_ptr<Config> config)
      : roleArn_(config->roleArn()),
        roleSessionName_(config->roleSessionName()),
        policy_(config->hasPolicy()
                    ? std::make_shared<std::string>(config->policy())
                    : nullptr),
        durationSeconds_(config->durationSeconds()), regionId_(config->regionId()),
        stsEndpoint_(config->stsEndpoint()) {
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

protected:
  virtual bool refreshCredential() const override;

  mutable Credential credential_;
  std::string roleArn_;
  std::string roleSessionName_;
  std::shared_ptr<std::string> policy_ = nullptr;
  int64_t durationSeconds_ = 3600;
  std::string regionId_ = "cn-hangzhou";
  std::string stsEndpoint_ = "sts.aliyuncs.com";
};

} // namespace Credential

} // namespace Alibabacloud

#endif