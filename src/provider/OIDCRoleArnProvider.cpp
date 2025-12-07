#include <fstream>

#include <darabonba/Exception.hpp>
#include <darabonba/Core.hpp>

#include <alibabacloud/credential/AuthUtil.hpp>
#include <alibabacloud/credential/provider/OIDCRoleArnProvider.hpp>

namespace AlibabaCloud {
namespace Credential {
bool OIDCRoleArnProvider::refreshCredential() const {
  std::ifstream ifs(oidcTokenFilePath_);
  if (!ifs) {
    throw Darabonba::Exception("Can't open " + oidcTokenFilePath_);
  }
  std::string oidcToken((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
  ifs.close();

  // V3 format: business parameters in Query
  Darabonba::Http::Query query = {
      {"DurationSeconds", std::to_string(durationSeconds_)},
      {"RoleArn", roleArn_},
      {"OIDCProviderArn", oidcProviderArn_},
      {"OIDCToken", oidcToken},
      {"RoleSessionName", roleSessionName_},
  };
  if (policy_) {
    query.emplace("Policy", *policy_);
  }

  // Build request
  std::string url = "https://" + stsEndpoint_ + "/";
  auto req = AuthUtil::getNewRequest(url);
  req.setQuery(std::move(query));
  req.setMethod("POST");

  // V3 format: common parameters in Header
  std::string utcDate = gmt_datetime();
  std::string nonce = Darabonba::Core::uuid();

  req.header()["host"] = stsEndpoint_;
  req.header()["x-acs-action"] = "AssumeRoleWithOIDC";
  req.header()["x-acs-version"] = "2015-04-01";
  req.header()["x-acs-date"] = utcDate;
  req.header()["x-acs-signature-nonce"] = nonce;
  // OIDC does not require signature, so no need to add Authorization Header

  // Use saved timeout configuration
  Darabonba::RuntimeOptions runtime;
  runtime.setConnectTimeout(connectTimeout_);
  runtime.setReadTimeout(readTimeout_);
  auto future = Darabonba::Core::doAction(req, runtime);
  auto resp = future.get();
  if (resp->statusCode() != 200) {
    throw Darabonba::Exception(Darabonba::Stream::readAsString(resp->body()));
  }
  auto result = Darabonba::Stream::readAsJSON(resp->body());
  auto &credential = result["Credentials"];
  this->expiration_ = strtotime(credential["Expiration"].get<std::string>());
  credential_.setAccessKeyId(credential["AccessKeyId"].get<std::string>())
      .setAccessKeySecret(credential["AccessKeySecret"].get<std::string>())
      .setSecurityToken(credential["SecurityToken"].get<std::string>());
  return true;
}

} // namespace Credential
} // namespace AlibabaCloud