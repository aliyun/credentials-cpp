#ifndef ALIBABACLOUD_CREDENTIAL_RSAKEYPAIRPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_RSAKEYPAIRPROVIDER_HPP_

#include <string>

#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/provider/NeedFreshProvider.hpp>

namespace AlibabaCloud {
namespace Credential {

class RsaKeyPairProvider : public NeedFreshProvider,
                           std::enable_shared_from_this<RsaKeyPairProvider> {
public:
  RsaKeyPairProvider(std::shared_ptr<Models::Config> config)
      : durationSeconds_(config->getDurationSeconds()),
        regionId_(config->getRegionId()), stsEndpoint_(config->getStsEndpoint()) {
    credential_.setAccessKeyId(config->getAccessKeyId())
        .setAccessKeySecret(config->getAccessKeySecret())
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
  
  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override { return Constant::RSA_KEY_PAIR; }

protected:
  virtual bool refreshCredential() const override ;

  int64_t durationSeconds_ = 3600;
  std::string regionId_ = "cn-hangzhou";
  std::string stsEndpoint_ = "sts.aliyuncs.com";
};

} // namespace Credential

} // namespace AlibabaCloud

#endif