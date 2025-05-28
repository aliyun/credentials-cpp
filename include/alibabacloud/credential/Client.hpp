#ifndef ALIBABACLOUD_CREDENTIAL_CLIENT_HPP_
#define ALIBABACLOUD_CREDENTIAL_CLIENT_HPP_

#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/provider/Provider.hpp>
// #include <darabonba/Model.hpp>
using namespace AlibabaCloud::Credential::Models;

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
  Client() = default;
  Client(const Config &obj)
      : config_(std::make_shared<Config>(obj)),
        provider_(makeProvider(config_)) {}
  Client(Config &&obj)
      : config_(std::make_shared<Config>(std::move(obj))),
        provider_(makeProvider(config_)) {}
  Client(std::shared_ptr<Config> config)
      : config_(config), provider_(makeProvider(config_)) {}
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
  std::string getType() { return provider_->getCredential().type(); }

  /**
   * @note Return a copy to avoid inconsistencies
   */
  CredentialModel getCredential() const { return provider_->getCredential(); }

private:
  static std::shared_ptr<Provider> makeProvider(std::shared_ptr<Config> config);

  std::shared_ptr<Config> config_ = nullptr;

  // using shared_ptr to enable copy
  std::shared_ptr<Provider> provider_ = nullptr;
};

} // namespace Credential
} // namespace Alibabacloud

#endif