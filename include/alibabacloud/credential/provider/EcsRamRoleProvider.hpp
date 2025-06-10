#ifndef AlibabaCloud_CREDENTIAL_ECSRAMROLEPROVIDER_HPP_
#define AlibabaCloud_CREDENTIAL_ECSRAMROLEPROVIDER_HPP_
#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/provider/NeedFreshProvider.hpp>
#include <memory>
#include <string>

namespace AlibabaCloud {
namespace Credential {

class EcsRamRoleProvider : public NeedFreshProvider,
                           std::enable_shared_from_this<EcsRamRoleProvider> {
public:
  EcsRamRoleProvider(std::shared_ptr<Models::Config> config)
      : roleName_(config->roleName()) {
    credential_.setType(Constant::ECS_RAM_ROLE);
  }

  EcsRamRoleProvider(const std::string &roleName) : roleName_(roleName) {
    credential_.setType(Constant::ECS_RAM_ROLE);
  }
  virtual ~EcsRamRoleProvider() {}

private:
  virtual bool refreshCredential() const override;
  static std::string getRoleName();

  static const std::string URL_IN_ECS_META_DATA;
  static const std::string ECS_META_DATA_FETCH_ERROR_MSG;
  static const std::string META_DATA_SERVICE_HOST;
  static const std::string ECS_METADATA_FETCH_ERROR_MSG;

  mutable Models::CredentialModel credential_;
  mutable std::string roleName_;
};

} // namespace Credential

} // namespace AlibabaCloud
#endif