#ifndef ALIBABACLOUD_CREDENTIAL_CLIENT_HPP_
#define ALIBABACLOUD_CREDENTIAL_CLIENT_HPP_

#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/provider/Provider.hpp>

#include <memory>
#include <string>

namespace AlibabaCloud {
namespace Credential {
class Client : public Darabonba::Model {
  friend void to_json(Darabonba::Json &j, const Client &obj) {
    DARABONBA_PTR_TO_JSON(config, config_);
  }
  friend void from_json(const Darabonba::Json &j, Client &obj) {
    DARABONBA_PTR_FROM_JSON(config, config_);
    obj.provider_ = makeProvider(obj.config_);
  }

public:
  // Constructor 1: Default constructor
  // Equivalent to Java: public Client()
  Client();
  
  // Constructor 2: Config-based constructors (3 overloads for different config types)
  // Equivalent to Java: public Client(Config config)
  Client(const Models::Config &obj);
  Client(Models::Config &&obj);
  Client(std::shared_ptr<Models::Config> config);
  
  // Constructor 3: Provider-based constructor
  // Equivalent to Java: public Client(AlibabaCloudCredentialsProvider provider)
  Client(std::shared_ptr<Provider> provider);
  
  virtual ~Client() = default;

  virtual void validate() const override {}

  virtual void fromMap(const Darabonba::Json &obj) override {
    from_json(obj, *this);
    validate();
  }

  virtual Darabonba::Json toMap() const override {
    Darabonba::Json obj;
    to_json(obj, *this);
    return obj;
  }

  virtual bool empty() const override {
    // a special implementation
    return provider_ == nullptr;
  }

  // Deprecated: use getCredentials() to avoid AK/SK misalignment due to refresh
  std::string getAccessKeyId() {
    return provider_->getCredential().accessKeyId();
  };
  std::string getAccessKeySecret() {
    return provider_->getCredential().accessKeySecret();
  }
  std::string getSecurityToken() {
    return provider_->getCredential().securityToken();
  }
  std::string getBearerToken() {
    return provider_->getCredential().bearerToken();
  }
  std::string getType() { return provider_->getProviderName(); }

  /**
   * @note Return a copy to avoid inconsistencies
   */
  Models::CredentialModel getCredential() const { return provider_->getCredential(); }

private:
  static std::shared_ptr<Provider> makeProvider(std::shared_ptr<Models::Config> config);

  std::shared_ptr<Models::Config> config_ = nullptr;

  // using shared_ptr to enable copy
  std::shared_ptr<Provider> provider_ = nullptr;
};

} // namespace Credential
} // namespace Alibabacloud



namespace nlohmann {
  template <>
  struct adl_serializer<std::shared_ptr<AlibabaCloud::Credential::Client>> {
    static void to_json(json &j, const std::shared_ptr<AlibabaCloud::Credential::Client> client) {
      j = reinterpret_cast<uintptr_t>(client.get());
    }

    static std::shared_ptr<AlibabaCloud::Credential::Client> from_json(const json &j) {
      if (!j.is_null()) {
        AlibabaCloud::Credential::Client *ptr = reinterpret_cast<AlibabaCloud::Credential::Client *>(j.get<uintptr_t>());
        return std::make_shared<AlibabaCloud::Credential::Client>(*ptr);
      }
      return nullptr;
    }
  };
}

#endif