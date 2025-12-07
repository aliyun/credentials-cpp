#include <alibabacloud/credential/AuthUtil.hpp>
#include <alibabacloud/credential/provider/CloudSSOCredentialsProvider.hpp>
#include <darabonba/Core.hpp>
#include <darabonba/http/Query.hpp>
#include <memory>

namespace AlibabaCloud {
namespace Credential {

const std::string CloudSSOCredentialsProvider::CLOUD_SSO_ENDPOINT =
    "cloudsso.aliyuncs.com";
const std::string CloudSSOCredentialsProvider::CLOUD_SSO_FETCH_ERROR_MSG =
    "Failed to get credentials from Cloud SSO service.";

bool CloudSSOCredentialsProvider::refreshCredential() const {
  Darabonba::Http::Query query = {
      {"Action", "GetRoleCredentials"},
      {"Format", "JSON"},
      {"Version", "2021-05-15"},
      {"RegionId", regionId_},
      {"RoleName", roleName_},
      {"Timestamp", gmt_datetime()},
      {"SignatureNonce", Darabonba::Core::uuid()},
  };

  // 使用 getNewRequest 创建带 User-Agent 的请求（对应 Python SDK）
  std::string url = "https://" + CLOUD_SSO_ENDPOINT + "/";
  auto req = AuthUtil::getNewRequest(url);
  req.setQuery(std::move(query));

  // 使用保存的超时配置
  Darabonba::RuntimeOptions runtime;
  runtime.setConnectTimeout(connectTimeout_);
  runtime.setReadTimeout(readTimeout_);
  auto future = Darabonba::Core::doAction(req, runtime);
  auto resp = future.get();
  if (resp->statusCode() != 200) {
    throw Darabonba::Exception(CLOUD_SSO_FETCH_ERROR_MSG + " Status code is " +
                               std::to_string(resp->statusCode()) +
                               ". Body is " +
                               Darabonba::Stream::readAsString(resp->body()));
  }

  auto result = Darabonba::Stream::readAsJSON(resp->body());
  if (result.contains("Code") &&
      result["Code"].get<std::string>() != "Success") {
    throw Darabonba::Exception(CLOUD_SSO_FETCH_ERROR_MSG +
                               " Response: " + result.dump());
  }

  auto &credential = result["RoleCredentials"];
  std::string accessKeyId = credential["AccessKeyId"].get<std::string>();
  std::string accessKeySecret =
      credential["AccessKeySecret"].get<std::string>();
  std::string securityToken = credential["SecurityToken"].get<std::string>();
  auto expiration = strtotime(credential["Expiration"].get<std::string>());

  this->expiration_ = expiration;
  credential_.setAccessKeyId(accessKeyId)
      .setAccessKeySecret(accessKeySecret)
      .setSecurityToken(securityToken);
  return true;
}

} // namespace Credential
} // namespace AlibabaCloud
