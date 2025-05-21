#include <alibabacloud/provider/OIDCRoleArnProvider.hpp>
#include <darabonba/Core.hpp>
#include <darabonba/Util.hpp>
#include <fstream>

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
  Darabonba::Http::Query query = {
      {"Action", "AssumeRoleWithOIDC"},
      {"Format", "JSON"},
      {"Version", "2015-04-01"},
      {"RegionId", regionId_},
      {"DurationSeconds", std::to_string(durationSeconds_)},
      {"RoleArn", roleArn_},
      {"OIDCProviderArn", oidcProviderArn_},
      {"OIDCToken", oidcToken},
      {"RoleSessionName", roleSessionName_},
      {"Timestamp", gmt_datetime()},
      {"SignatureNonce", Darabonba::Core::uuid()},
  };
  if(policy_) {
    query.emplace("Policy", *policy_);
  }
  Darabonba::Http::Request req;
  req.url().setScheme("https");
  req.header()["host"] = stsEndpoint_;
  req.setQuery(std::move(query));

  auto future = Darabonba::Core::doAction(req);
  auto resp = future.get();
  if (resp->statusCode() != 200) {
    throw Darabonba::Exception(Darabonba::Util::readAsString(resp->body()));
  }
  auto result = Darabonba::Util::readAsJSON(resp->body());
  auto &credential = result["Credentials"];
  this->expiration_ = strtotime(credential["Expiration"].get<std::string>());
  credential_.setAccessKeySecret(credential["AccessKeyId"].get<std::string>())
      .setAccessKeySecret(credential["AccessKeySecret"].get<std::string>())
      .setSecurityToken(credential["SecurityToken"].get<std::string>());
  return true;
}
} // namespace Credential
} // namespace AlibabaCloud