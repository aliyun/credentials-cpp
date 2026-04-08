#include <memory>

#include <darabonba/Core.hpp>
#include <darabonba/encode/Encoder.hpp>
#include <darabonba/encode/SHA256.hpp>
#include <darabonba/http/Query.hpp>
#include <darabonba/signature/Signer.hpp>

#include <alibabacloud/credentials/AuthUtil.hpp>
#include <alibabacloud/credentials/Exception.hpp>
#include <alibabacloud/credentials/provider/RamRoleArnProvider.hpp>
#include <darabonba/Core.hpp>
#include <darabonba/http/Query.hpp>
#include <memory>

namespace AlibabaCloud {
namespace Credentials {

RamRoleArnProvider::RamRoleArnProvider(std::shared_ptr<Models::Config> config,
                                       StaleValueBehavior behavior,
                                       std::shared_ptr<PrefetchStrategy> strategy)
    : RefreshableProvider(behavior, strategy),
      accessKeyId_(config->getAccessKeyId()),
      accessKeySecret_(config->getAccessKeySecret()),
      roleArn_(config->getRoleArn()),
      roleSessionName_(config->hasRoleSessionName() && !config->getRoleSessionName().empty()
                           ? config->getRoleSessionName()
                           : "credentials-cpp-session"),
      policy_(config->hasPolicy() && !config->getPolicy().empty()
                  ? std::make_shared<std::string>(config->getPolicy())
                  : nullptr),
      durationSeconds_(config->hasRoleSessionExpiration() && config->getRoleSessionExpiration() > 0
                           ? config->getRoleSessionExpiration()
                           : 3600),
      regionId_(config->hasRegionId() && !config->getRegionId().empty()
                    ? config->getRegionId()
                    : "cn-hangzhou"),
      stsEndpoint_(config->hasStsEndpoint() && !config->getStsEndpoint().empty()
                       ? config->getStsEndpoint()
                       : "sts.aliyuncs.com"),
      enableVpc_(config->hasEnableVpc() ? config->getEnableVpc() : false),
      connectTimeout_(config->hasConnectTimeout() ? config->getConnectTimeout() : 10000),
      readTimeout_(config->hasTimeout() ? config->getTimeout() : 5000) {
  if (roleArn_.empty()) {
    throw CredentialException(std::string("RoleArn cannot be empty."));
  }
}

RamRoleArnProvider::RamRoleArnProvider(const std::string &accessKeyId,
                                       const std::string &accessKeySecret,
                                       const std::string &roleArn,
                                       const std::string &roleSessionName,
                                       std::shared_ptr<std::string> policy,
                                       int64_t durationSeconds,
                                       const std::string &regionId,
                                       const std::string &stsEndpoint,
                                       StaleValueBehavior behavior,
                                       std::shared_ptr<PrefetchStrategy> strategy)
    : RefreshableProvider(behavior, strategy),
      accessKeyId_(accessKeyId),
      accessKeySecret_(accessKeySecret),
      roleArn_(roleArn),
      roleSessionName_(roleSessionName.empty() ? "credentials-cpp-session" : roleSessionName),
      policy_(policy),
      durationSeconds_(durationSeconds > 0 ? durationSeconds : 3600),
      regionId_(regionId.empty() ? "cn-hangzhou" : regionId),
      stsEndpoint_(stsEndpoint.empty() ? "sts.aliyuncs.com" : stsEndpoint),
      enableVpc_(false),
      connectTimeout_(10000),
      readTimeout_(5000) {
  if (roleArn_.empty()) {
    throw CredentialException(std::string("RoleArn cannot be empty."));
  }
}

int64_t RamRoleArnProvider::getStaleTime(int64_t expiration) const {
  if (expiration <= 0) {
    return getCurrentTime() + 60 * 60;  // Default 1 hour if no expiration
  }
  return expiration - STALE_TIME_WINDOW;  // expiration - 15 minutes
}

RefreshResult RamRoleArnProvider::doRefresh() const {
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

  req.getHeaders()["host"] = stsEndpoint_;
  req.getHeaders()["x-acs-action"] = "AssumeRole";
  req.getHeaders()["x-acs-version"] = "2015-04-01";
  req.getHeaders()["x-acs-date"] = utcDate;
  req.getHeaders()["x-acs-signature-nonce"] = nonce;
  req.getHeaders()["x-acs-content-sha256"] =
      "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b9"
      "34ca495991b7852b855"; // SHA256 of empty body

  // Build canonical request string
  std::string canonicalQueryString = std::string(req.getQuery());
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
          stringToSign, accessKeySecret_));

  // Build Authorization Header
  std::string authorization =
      "ACS3-HMAC-SHA256 Credential=" + accessKeyId_ +
      ",SignedHeaders=" + signedHeaders + ",Signature=" + signature;
  req.getHeaders()["Authorization"] = authorization;

  // Use saved timeout configuration
  Darabonba::RuntimeOptions runtime;
  runtime.setConnectTimeout(connectTimeout_);
  runtime.setReadTimeout(readTimeout_);
  auto future = Darabonba::Core::doAction(req, runtime);
  auto resp = future.get();
  if (resp->getStatusCode() != 200) {
    throw CredentialException(Darabonba::Stream::readAsString(resp->getBody()));
  }
  auto result = Darabonba::Stream::readAsJSON(resp->getBody());
  if (result["Code"].get<std::string>() != "Success") {
    throw CredentialException(result.dump());
  }
  auto &credential = result["Credentials"];
  int64_t expiration = strtotime(credential["Expiration"].get<std::string>());

  // Build credential model
  Models::CredentialModel cred;
  cred.setAccessKeyId(credential["AccessKeyId"].get<std::string>())
      .setAccessKeySecret(credential["AccessKeySecret"].get<std::string>())
      .setSecurityToken(credential["SecurityToken"].get<std::string>())
      .setType(Constant::RAM_ROLE_ARN)
      .setProviderName(getProviderName());

  // Calculate stale time (expiration - 15 minutes)
  int64_t staleTime = getStaleTime(expiration);
  // Calculate prefetch time (stale time - 180 seconds)
  int64_t prefetchTime = staleTime - PREFETCH_THRESHOLD;

  return RefreshResult(cred, staleTime, prefetchTime);
}

} // namespace Credentials
} // namespace AlibabaCloud