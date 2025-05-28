#include <cstdint>
#include <darabonba/Core.hpp>
#include <darabonba/Util.hpp>
#include <darabonba/http/Query.hpp>
#include <darabonba/http/URL.hpp>
#include <darabonba/signature/Signer.hpp>
#include <alibabacloud/credential/provider/RamRoleArnProvider.hpp>
#include <memory>

namespace AlibabaCloud {
namespace Credential {

bool RamRoleArnProvider::refreshCredential()  {
  Darabonba::Http::Query query = {
      {"Action", "AssumeRole"},
      {"Format", "JSON"},
      {"Version", "2015-04-01"},
      {"DurationSeconds", std::to_string(durationSeconds_)},
      {"RoleArn", roleArn_},
      {"AccessKeyId", credential_.accessKeyId()},
      {"RegionId", regionId_},
      {"RoleSessionName", roleSessionName_},
      {"SignatureMethod", "HMAC-SHA1"},
      {"SignatureVersion", "1.0"},
      {"Timestamp", gmt_datetime()},
      {"SignatureNonce", Darabonba::Core::uuid()},
  };
  if (policy_) {
    query.emplace("Policy", *policy_);
  }

  // %2F is the url_encode of '/'
  std::string stringToSign = "GET&%2F&" + std::string(query);
  std::string signature =
      Darabonba::Util::toString(Darabonba::Signature::Signer::HmacSHA1Sign(
          stringToSign, credential_.accessKeySecret()));
  query.emplace("Signature", signature);

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
  if (result["Code"].get<std::string>() != "Success") {
    throw Darabonba::Exception(result.dump());
  }
  auto &credential = result["Credentials"];
  this->expiration_ = strtotime(credential["Expiration"].get<std::string>());
  credential_.setAccessKeySecret(credential["AccessKeyId"].get<std::string>())
      .setAccessKeySecret(credential["AccessKeySecret"].get<std::string>())
      .setSecurityToken(credential["SecurityToken"].get<std::string>());
  return true;
}

} // namespace Credential
} // namespace AlibabaCloud