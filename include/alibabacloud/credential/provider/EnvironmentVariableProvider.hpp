#ifndef AlibabaCloud_CREDENTIAL_ENVIRONMENTVARIABLEPROVIDER_HPP_
#define AlibabaCloud_CREDENTIAL_ENVIRONMENTVARIABLEPROVIDER_HPP_
#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/provider/Provider.hpp>
#include <memory>
#include <darabonba/Exception.hpp>
namespace AlibabaCloud {

namespace Credential {

class EnvironmentVariableProvider : public Provider {
public:
  EnvironmentVariableProvider() = default;
  virtual ~EnvironmentVariableProvider() {}

  virtual Models::CredentialModel &getCredential() override {
    provider_ = createProvider();
    if (provider_ == nullptr) {
      throw Darabonba::Exception("Can't create the ProfileProvider.");
    }
    return provider_->getCredential();
  }

  virtual const Models::CredentialModel &getCredential() const override {
    provider_ = createProvider();
    if (provider_ == nullptr) {
      throw Darabonba::Exception("Can't create the ProfileProvider.");
    }
    return provider_->getCredential();
  }

protected:
  static std::unique_ptr<Provider> createProvider();

  mutable std::unique_ptr<Provider> provider_ = nullptr;
  mutable Models::CredentialModel credential_;
};

} // namespace Credential
} // namespace AlibabaCloud

#endif
