#ifndef AlibabaCloud_CREDENTIAL_PROFILEPROVIDER_HPP_
#define AlibabaCloud_CREDENTIAL_PROFILEPROVIDER_HPP_
#include <alibabacloud/credential/provider/Provider.hpp>
#include <darabonba/Exception.hpp>
#include <memory>
namespace AlibabaCloud {

namespace Credential {
class ProfileProvider : public Provider {
public:
  // ProfileProvider() = default;
  // virtual ~ProfileProvider() = default;

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
};

} // namespace Credential
} // namespace AlibabaCloud

#endif