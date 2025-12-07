#include <memory>

#include <darabonba/Core.hpp>
#include <darabonba/encode/Encoder.hpp>
#include <darabonba/encode/SHA256.hpp>
#include <darabonba/http/Query.hpp>
#include <darabonba/signature/Signer.hpp>

#include <alibabacloud/credential/AuthUtil.hpp>
#include <alibabacloud/credential/provider/RamRoleArnProvider.hpp>

namespace AlibabaCloud {
namespace Credential {

bool RamRoleArnProvider::refreshCredential() const {
  Darabonba::Http::Query query = {
      {"DurationSeconds", std::to_string(durationSeconds_)},
      {"RoleArn", roleArn_},
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

  // V3 signature: common parameters in Header
  std::string utcDate = gmt_datetime();
  std::string nonce = Darabonba::Core::uuid();

  req.header()["host"] = stsEndpoint_;
  req.header()["x-acs-action"] = "AssumeRole";
  req.header()["x-acs-version"] = "2015-04-01";
  req.header()["x-acs-date"] = utcDate;
  req.header()["x-acs-signature-nonce"] = nonce;
  req.header()["x-acs-content-sha256"] =
      "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b9"
      "34ca495991b7852b855"; // SHA256 of empty body

  // Build canonical request string
  std::string canonicalQueryString = std::string(req.query());
  std::string canonicalHeaders =
      "host:" + stsEndpoint_ + "\n" + "x-acs-action:AssumeRole\n" +
      "x-acs-content-sha256:"
      "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n" +
      "x-acs-date:" + utcDate + "\n" + "x-acs-signature-nonce:" + nonce + "\n" +
      "x-acs-version:2015-04-01";
  std::string signedHeaders = "host;x-acs-action;x-acs-content-sha256;x-acs-"
                              "date;x-acs-signature-nonce;x-acs-version";
  std::string hashedRequestPayload = "e3b0c44298fc1c149afbf4c8996fb92427ae41e46"
                                     "49b934ca495991b7852b855"; // empty body

  std::string canonicalRequest = "POST\n/\n" + canonicalQueryString + "\n" +
                                 canonicalHeaders + "\n\n" + signedHeaders +
                                 "\n" + hashedRequestPayload;

  // Calculate hash of canonical request
  Darabonba::Encode::SHA256 sha256;
  auto hashedBytes =
      Darabonba::Encode::SHA256::hash(canonicalRequest.c_str(), canonicalRequest.size());
  std::string hashedCanonicalRequest =
      Darabonba::Encode::Encoder::hexEncode(hashedBytes);

  // Build string to sign
  std::string stringToSign = "ACS3-HMAC-SHA256\n" + hashedCanonicalRequest;

  // Calculate signature
  std::string signature = Darabonba::Encode::Encoder::hexEncode(
      Darabonba::Signature::Signer::HmacSHA256Sign(
          stringToSign, credential_.accessKeySecret()));

  // Build Authorization Header
  std::string authorization =
      "ACS3-HMAC-SHA256 Credential=" + credential_.accessKeyId() +
      ",SignedHeaders=" + signedHeaders + ",Signature=" + signature;
  req.header()["Authorization"] = authorization;

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
  if (result["Code"].get<std::string>() != "Success") {
    throw Darabonba::Exception(result.dump());
  }
  auto &credential = result["Credentials"];
  this->expiration_ = strtotime(credential["Expiration"].get<std::string>());
  credential_.setAccessKeyId(credential["AccessKeyId"].get<std::string>())
      .setAccessKeySecret(credential["AccessKeySecret"].get<std::string>())
      .setSecurityToken(credential["SecurityToken"].get<std::string>());
  return true;
}

} // namespace Credential
} // namespace AlibabaCloud