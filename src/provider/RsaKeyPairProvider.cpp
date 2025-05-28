#include <alibabacloud/credential/provider/RsaKeyPairProvider.hpp>
#include <darabonba/Core.hpp>
#include <darabonba/Util.hpp>
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
       {"AccessKeyId", credential_.accessKeyId()},
       {"RegionId", regionId_},
       {"SignatureMethod", "HMAC-SHA1"},
       {"SignatureVersion", "1.0"},
       {"Timestamp", gmt_datetime()},
       {"SignatureNonce", Darabonba::Core::uuid()},
   };

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