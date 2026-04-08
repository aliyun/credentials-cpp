#include <fstream>

#include <alibabacloud/credentials/AuthUtil.hpp>
#include <alibabacloud/credentials/Exception.hpp>
#include <alibabacloud/credentials/provider/RsaKeyPairProvider.hpp>
#include <darabonba/Core.hpp>
#include <darabonba/encode/Encoder.hpp>
#include <darabonba/http/Query.hpp>
#include <darabonba/signature/Signer.hpp>

namespace AlibabaCloud {
namespace Credentials {

// Helper function to read private key from file
static std::string readPrivateKeyFromFile(const std::string& filePath) {
  if (filePath.empty()) {
    return "";
  }
  std::ifstream ifs(filePath);
  if (!ifs) {
    throw CredentialException(std::string("Cannot open private key file: ") + filePath);
  }
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
  return content;
}

RsaKeyPairProvider::RsaKeyPairProvider(std::shared_ptr<Models::Config> config,
                                       StaleValueBehavior behavior,
                                       std::shared_ptr<PrefetchStrategy> strategy)
    : RefreshableProvider(behavior, strategy),
      publicKeyId_(config->getPublicKeyId()),
      privateKey_(config->hasPrivateKeyFile() ? readPrivateKeyFromFile(config->getPrivateKeyFile()) : ""),
      durationSeconds_(config->getDurationSeconds() > 0 ? config->getDurationSeconds() : 3600),
      regionId_(config->getRegionId().empty() ? "cn-hangzhou" : config->getRegionId()),
      stsEndpoint_(config->getStsEndpoint().empty() ? "sts.aliyuncs.com" : config->getStsEndpoint()),
      connectTimeout_(config->hasConnectTimeout() ? config->getConnectTimeout() : 5000),
      readTimeout_(config->hasTimeout() ? config->getTimeout() : 10000) {
  if (publicKeyId_.empty()) {
    throw CredentialException(std::string("PublicKeyId cannot be empty."));
  }
  if (privateKey_.empty()) {
    throw CredentialException(std::string("PrivateKey cannot be empty."));
  }
}

RsaKeyPairProvider::RsaKeyPairProvider(const std::string &publicKeyId,
                                       const std::string &privateKey,
                                       int64_t durationSeconds,
                                       const std::string &regionId,
                                       const std::string &stsEndpoint,
                                       StaleValueBehavior behavior,
                                       std::shared_ptr<PrefetchStrategy> strategy)
    : RefreshableProvider(behavior, strategy),
      publicKeyId_(publicKeyId),
      privateKey_(privateKey),
      durationSeconds_(durationSeconds > 0 ? durationSeconds : 3600),
      regionId_(regionId.empty() ? "cn-hangzhou" : regionId),
      stsEndpoint_(stsEndpoint.empty() ? "sts.aliyuncs.com" : stsEndpoint),
      connectTimeout_(5000),
      readTimeout_(10000) {
  if (publicKeyId_.empty()) {
    throw CredentialException(std::string("PublicKeyId cannot be empty."));
  }
  if (privateKey_.empty()) {
    throw CredentialException(std::string("PrivateKey cannot be empty."));
  }
}

int64_t RsaKeyPairProvider::getStaleTime(int64_t expiration) const {
  if (expiration <= 0) {
    return getCurrentTime() + 60 * 60;  // Default 1 hour if no expiration
  }
  return expiration - STALE_TIME_WINDOW;  // expiration - 15 minutes
}

RefreshResult RsaKeyPairProvider::doRefresh() const {
  Darabonba::Http::Query query = {
      {"Action", "GenerateSessionAccessKey"},
      {"Format", "JSON"},
      {"Version", "2015-04-01"},
      {"DurationSeconds", std::to_string(durationSeconds_)},
      {"AccessKeyId", publicKeyId_},
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
          stringToSign, privateKey_));
  query.emplace("Signature", signature);

  // 使用 getNewRequest 创建带 User-Agent 的请求
  const std::string url = "https://" + stsEndpoint_ + "/";
  auto req = AuthUtil::getNewRequest(url);
  req.setQuery(std::move(query));

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
  auto &sessionAccessKey = result["SessionAccessKey"];
  int64_t expiration = strtotime(sessionAccessKey["Expiration"].get<std::string>());

  // Build credential model
  Models::CredentialModel cred;
  cred.setAccessKeyId(sessionAccessKey["SessionAccessKeyId"].get<std::string>())
      .setAccessKeySecret(sessionAccessKey["SessionAccessKeySecret"].get<std::string>())
      .setType(Constant::RSA_KEY_PAIR)
      .setProviderName(getProviderName());

  // Calculate stale time (expiration - 15 minutes)
  int64_t staleTime = getStaleTime(expiration);
  // Calculate prefetch time (stale time - 180 seconds)
  int64_t prefetchTime = staleTime - PREFETCH_THRESHOLD;

  return RefreshResult(cred, staleTime, prefetchTime);
}

} // namespace Credentials
} // namespace AlibabaCloud