#ifndef ALIBABACLOUD_CREDENTIAL_RSAKEYPAIRPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_RSAKEYPAIRPROVIDER_HPP_
#include <alibabacloud/credential/Config.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/provider/NeedFreshProvider.hpp>

#include <string>

namespace Alibabacloud {
namespace Credential {

class RsaKeyPairProvider : public NeedFreshProvider,
                           std::enable_shared_from_this<RsaKeyPairProvider> {
public:
  RsaKeyPairProvider(std::shared_ptr<Config> config)
      : durationSeconds_(config->durationSeconds()),
        regionId_(config->regionId()), stsEndpoint_(config->stsEndpoint()) {
    credential_.setAccessKeyId(config->accessKeyId())
        .setAccessKeySecret(config->accessKeySecret())
        .setType(Constant::RSA_KEY_PAIR);
  }
  RsaKeyPairProvider(const std::string &accessKeyId,
                     const std::string &accessKeySecret,
                     int64_t durationSeconds = 3600,
                     const std::string &regionId = "cn-hangzhou",
                     const std::string &stsEndpoint = "sts.aliyuncs.com")
      : durationSeconds_(durationSeconds), regionId_(regionId),
        stsEndpoint_(stsEndpoint) {
    credential_.setAccessKeyId(accessKeyId)
        .setAccessKeySecret(accessKeySecret)
        .setType(Constant::RSA_KEY_PAIR);
  }

  virtual ~RsaKeyPairProvider() {}

protected:
  virtual bool refreshCredential() const override;

  int64_t durationSeconds_ = 3600;
  std::string regionId_ = "cn-hangzhou";
  std::string stsEndpoint_ = "sts.aliyuncs.com";
};

} // namespace Credential

} // namespace Alibabacloud

#endif