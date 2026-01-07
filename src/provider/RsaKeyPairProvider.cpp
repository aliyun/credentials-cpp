#include <alibabacloud/credential/AuthUtil.hpp>
#include <alibabacloud/credential/provider/RsaKeyPairProvider.hpp>
#include <darabonba/Core.hpp>
#include <darabonba/encode/Encoder.hpp>
#include <darabonba/http/Query.hpp>
#include <darabonba/signature/Signer.hpp>

namespace AlibabaCloud {
namespace Credential {

bool RsaKeyPairProvider::refreshCredential() const {
  Darabonba::Http::Query query = {
      {"Action", "GenerateSessionAccessKey"},
      {"Format", "JSON"},
      {"Version", "2015-04-01"},
      {"DurationSeconds", std::to_string(durationSeconds_)},
      {"AccessKeyId", credential_.getAccessKeyId()},
      {"RegionId", regionId_},
      {"SignatureMethod", "HMAC-SHA1"},
      {"SignatureVersion", "1.0"},
      {"Timestamp", gmt_datetime()},
      {"SignatureNonce", Darabonba::Core::uuid()},
  };

  // %2F is the url_encode of '/'
  std::string stringToSign = "GET&%2F&" + std::string(query);
  std::string signature = Darabonba::Encode::Encoder::toString(
      Darabonba::Signature::Signer::HmacSHA1Sign(
          stringToSign, credential_.getAccessKeySecret()));
  query.emplace("Signature", signature);

  // 使用 getNewRequest 创建带 User-Agent 的请求（对应 Python SDK 的
  // getNewRequest）
  const std::string url = "https://" + stsEndpoint_ + "/";
  auto req = AuthUtil::getNewRequest(url);
  req.setQuery(std::move(query));

  auto future = Darabonba::Core::doAction(req);
  auto resp = future.get();
  if (resp->statusCode() != 200) {
    throw Darabonba::Exception(Darabonba::Stream::readAsString(resp->body()));
  }
  auto result = Darabonba::Stream::readAsJSON(resp->body());
  if (result["Code"].get<std::string>() != "Success") {
    throw Darabonba::Exception(result.dump());
  }
  auto &sessionAccessKey = result["SessionAccessKey"];
  this->expiration_ =
      strtotime(sessionAccessKey["Expiration"].get<std::string>());
  credential_
      .setAccessKeyId(sessionAccessKey["SessionAccessKeyId"].get<std::string>())
      .setAccessKeySecret(
          sessionAccessKey["SessionAccessKeySecret"].get<std::string>());
  return true;
}

} // namespace Credential
} // namespace AlibabaCloud