#ifndef AlibabaCloud_CREDENTIAL_ENVIRONMENTVARIABLEPROVIDER_HPP_
#define AlibabaCloud_CREDENTIAL_ENVIRONMENTVARIABLEPROVIDER_HPP_

#include <alibabacloud/Constant.hpp>
#include <alibabacloud/provider/Provider.hpp>
#include <memory>

namespace AlibabaCloud {

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
} // namespace AlibabaCloud

#endif
