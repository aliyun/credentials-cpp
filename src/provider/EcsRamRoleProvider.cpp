#include <alibabacloud/credential/provider/EcsRamRoleProvider.hpp>
#include <darabonba/Core.hpp>
#include <darabonba/Util.hpp>
#include <memory>

namespace AlibabaCloud {
namespace Credential {

const std::string EcsRamRoleProvider::URL_IN_ECS_META_DATA =
    "/latest/meta-data/ram/security-credentials/";
const std::string EcsRamRoleProvider::ECS_META_DATA_FETCH_ERROR_MSG =
    "Failed to get RAM session credentials from ECS metadata service.";
const std::string EcsRamRoleProvider::META_DATA_SERVICE_HOST =
    "100.100.100.200";
const std::string EcsRamRoleProvider::ECS_METADATA_FETCH_ERROR_MSG =
    "Failed to get RAM session credentials from ECS metadata service.";

bool EcsRamRoleProvider::refreshCredential() const {
  if (roleName_.empty()) {
    roleName_ = getRoleName();
  }
  std::string url =
      "http://" + META_DATA_SERVICE_HOST + URL_IN_ECS_META_DATA + roleName_;
  Darabonba::Http::Request req(url);
  auto future = Darabonba::Core::doAction(req);
  auto resp = future.get();
  if (resp->statusCode() != 200) {
    throw Darabonba::Exception(ECS_METADATA_FETCH_ERROR_MSG +
                               " Status code is " +
                               std::to_string(resp->statusCode()));
  }
  auto result = Darabonba::Util::readAsJSON(resp->body());
  if (result["Code"].get<std::string>() != "Success") {
    throw Darabonba::Exception(
        ECS_METADATA_FETCH_ERROR_MSG + " Status code is " +
        std::to_string(resp->statusCode()) + ". Body is " + result.dump());
  }
  std::string accessKeyId = result["AccessKeyId"].get<std::string>(),
              accessKeySecret = result["AccessKeySecret"].get<std::string>(),
              securityToken = result["SecurityToken"].get<std::string>();
  auto expiration = strtotime(result["Expiration"].get<std::string>());
  this->expiration_ = expiration;
  credential_.setAccessKeyId(accessKeyId)
      .setAccessKeySecret(accessKeySecret)
      .setSecurityToken(securityToken);
  return true;
}

std::string EcsRamRoleProvider::getRoleName() {
  std::string url = "http://" + META_DATA_SERVICE_HOST + URL_IN_ECS_META_DATA;
  Darabonba::Http::Request req(url);
  auto future = Darabonba::Core::doAction(req);
  auto resp = future.get();
  if (resp->statusCode() != 200) {
    throw Darabonba::Exception(
        ECS_METADATA_FETCH_ERROR_MSG + " Status code is " +
        std::to_string(resp->statusCode()) + ", Body is " +
        Darabonba::Util::readAsString(resp->body()));
  }

  return Darabonba::Util::readAsString(resp->body());
}

} // namespace Credential
} // namespace AlibabaCloud