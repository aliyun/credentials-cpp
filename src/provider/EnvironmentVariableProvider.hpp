#ifndef ALIBABACLOUD_CREDENTIAL_ENVIRONMENTVARIABLEPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_ENVIRONMENTVARIABLEPROVIDER_HPP_

#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/provider/Provider.hpp>
#include <memory>

namespace Alibabacloud {

namespace Credential {

class EnvironmentVariableProvider : public Provider {
public:
  EnvironmentVariableProvider() = default;
  virtual ~EnvironmentVariableProvider() {}

  virtual Credential &getCredential() override {
    provider_ = createProvider();
    if (provider_ == nullptr) {
      throw Darabonba::Exception("Can't create the ProfileProvider.");
    }
    return provider_->getCredential();
  }

  virtual const Credential &getCredential() const override {
    provider_ = createProvider();
    if (provider_ == nullptr) {
      throw Darabonba::Exception("Can't create the ProfileProvider.");
    }
    return provider_->getCredential();
  }

protected:
  static std::unique_ptr<Provider> createProvider();

  mutable std::unique_ptr<Provider> provider_ = nullptr;
  mutable Credential credential_;
};

} // namespace Credential
} // namespace Alibabacloud

#endif
