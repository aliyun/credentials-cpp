 #include <alibabacloud/credential/provider/URLProvider.hpp>
 #include <darabonba/Core.hpp>

 namespace AlibabaCloud {
 namespace Credential {
 bool URLProvider::refreshCredential() const {
   Darabonba::Http::Request req(url_);
   auto future = Darabonba::Core::doAction(req);
   auto resp = future.get();
   if (resp->statusCode() != 200) {
     throw Darabonba::Exception(Darabonba::Stream::readAsString(resp->body()));
   }
   const auto &result= Darabonba::Stream::readAsJSON(resp->body());
   if(result["Code"].get<std::string>() != "Success") {
     throw Darabonba::Exception(result.dump());
   }
   this->expiration_ = strtotime(result["Expiration"].get<std::string>());
   credential_.setAccessKeySecret(result["AccessKeyId"].get<std::string>())
       .setAccessKeySecret(result["AccessKeySecret"].get<std::string>())
       .setSecurityToken(result["SecurityToken"].get<std::string>());
   return true;
 }

 } // namespace Credential
 } // namespace AlibabaCloud