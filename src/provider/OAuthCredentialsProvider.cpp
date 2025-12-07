#include <alibabacloud/credential/AuthUtil.hpp>
#include <alibabacloud/credential/provider/OAuthCredentialsProvider.hpp>
#include <darabonba/Core.hpp>
#include <darabonba/encode/Encoder.hpp>
#include <darabonba/http/Query.hpp>
#include <memory>

namespace AlibabaCloud {
namespace Credential {

const std::string OAuthCredentialsProvider::OAUTH_FETCH_ERROR_MSG =
    "Failed to get credentials from OAuth token endpoint.";

bool OAuthCredentialsProvider::refreshCredential() const {
  // OAuth 2.0 Client Credentials flow

  // 使用 getNewRequest 创建带 User-Agent 的请求（对应 Python SDK）
  auto req = AuthUtil::getNewRequest(tokenEndpoint_);
  req.setMethod("POST");

  // Extract host from token endpoint
  size_t hostStart = tokenEndpoint_.find("://");
  if (hostStart != std::string::npos) {
    hostStart += 3;
  } else {
    hostStart = 0;
  }
  size_t hostEnd = tokenEndpoint_.find('/', hostStart);
  std::string host = (hostEnd != std::string::npos)
                         ? tokenEndpoint_.substr(hostStart, hostEnd - hostStart)
                         : tokenEndpoint_.substr(hostStart);
  std::string path =
      (hostEnd != std::string::npos) ? tokenEndpoint_.substr(hostEnd) : "/";

  req.header()["host"] = host;
  req.url().setHost(host);
  if (!path.empty() && path != "/") {
    Darabonba::Http::Query query;
    size_t queryPos = path.find('?');
    if (queryPos != std::string::npos) {
      std::string queryStr = path.substr(queryPos + 1);
      path = path.substr(0, queryPos);
      // Parse query string if needed
    }
  }

  req.header()["Content-Type"] = "application/x-www-form-urlencoded";

  // Build OAuth request body
  std::string body =
      "grant_type=client_credentials"
      "&client_id=" +
      Darabonba::Encode::Encoder::urlEncode(clientId_) +
      "&client_secret=" + Darabonba::Encode::Encoder::urlEncode(clientSecret_);

  // Use std::string directly instead of shared_ptr
  req.setBody(body);

  // 使用保存的超时配置
  Darabonba::RuntimeOptions runtime;
  runtime.setConnectTimeout(connectTimeout_);
  runtime.setReadTimeout(readTimeout_);
  auto future = Darabonba::Core::doAction(req, runtime);
  auto resp = future.get();

  if (resp->statusCode() != 200) {
    throw Darabonba::Exception(OAUTH_FETCH_ERROR_MSG + " Status code is " +
                               std::to_string(resp->statusCode()) +
                               ". Body is " +
                               Darabonba::Stream::readAsString(resp->body()));
  }

  auto result = Darabonba::Stream::readAsJSON(resp->body());

  if (result.contains("error")) {
    throw Darabonba::Exception(
        OAUTH_FETCH_ERROR_MSG +
        " Error: " + result["error"].get<std::string>() +
        (result.contains("error_description")
             ? ", Description: " +
                   result["error_description"].get<std::string>()
             : ""));
  }

  // Extract OAuth token - typically access_token
  auto accessToken = result["access_token"].get<std::string>();

  // Calculate expiration time
  int64_t expiresIn = result.contains("expires_in")
                          ? result["expires_in"].get<int64_t>()
                          : 3600;
  this->expiration_ = static_cast<int64_t>(time(nullptr)) + expiresIn;

  // For OAuth, we store the access token as a bearer token
  credential_.setBearerToken(accessToken);

  // If the response includes additional credentials (some OAuth
  // implementations)
  if (result.contains("access_key_id")) {
    credential_.setAccessKeyId(result["access_key_id"].get<std::string>());
  }
  if (result.contains("access_key_secret")) {
    credential_.setAccessKeySecret(
        result["access_key_secret"].get<std::string>());
  }
  if (result.contains("security_token")) {
    credential_.setSecurityToken(result["security_token"].get<std::string>());
  }

  return true;
}

} // namespace Credential
} // namespace AlibabaCloud
