#ifndef ALIBABACLOUD_CREDENTIAL_CLOUDSSOCREDENTIALSPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_CLOUDSSOCREDENTIALSPROVIDER_HPP_

#include <memory>
#include <string>

#include <darabonba/Env.hpp>

#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/provider/NeedFreshProvider.hpp>

namespace AlibabaCloud {
namespace Credential {

class CloudSSOCredentialsProvider : public NeedFreshProvider,
                                     std::enable_shared_from_this<CloudSSOCredentialsProvider> {
public:
  CloudSSOCredentialsProvider(std::shared_ptr<Models::Config> config)
      : roleName_(config->hasRoleName() && !config->roleName().empty()
                      ? config->roleName()
                      : Darabonba::Env::getEnv(Constant::ENV_CLOUD_SSO_ROLE_NAME)),
        regionId_(config->regionId()),
        connectTimeout_(config->hasConnectTimeout() ? config->connectTimeout() : 10000),
        readTimeout_(config->hasTimeout() ? config->timeout() : 5000) {
    credential_.setType(Constant::CLOUD_SSO);
  }

  CloudSSOCredentialsProvider(const std::string &roleName,
                               const std::string &regionId = "cn-hangzhou")
      : roleName_(roleName), regionId_(regionId) {
    credential_.setType(Constant::CLOUD_SSO);
  }

  virtual ~CloudSSOCredentialsProvider() {}
  
  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override { return Constant::CLOUD_SSO; }

private:
  virtual bool refreshCredential() const override;

  static const std::string CLOUD_SSO_ENDPOINT;
  static const std::string CLOUD_SSO_FETCH_ERROR_MSG;

  mutable Models::CredentialModel credential_;
  std::string roleName_;
  std::string regionId_ = "cn-hangzhou";
  int64_t connectTimeout_ = 10000;  // Connection timeout in milliseconds
  int64_t readTimeout_ = 5000;      // Read timeout in milliseconds
};

} // namespace Credential
} // namespace AlibabaCloud

#endif
