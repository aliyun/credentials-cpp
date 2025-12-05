#ifndef ALIBABACLOUD_CREDENTIAL_PROFILEPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_PROFILEPROVIDER_HPP_

#include <memory>

#include <darabonba/Exception.hpp>

#include <alibabacloud/credential/provider/Provider.hpp>
namespace AlibabaCloud {
namespace Credential {
class ProfileProvider : public Provider {
public:

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
  
  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override {
    provider_ = createProvider();
    if (provider_ == nullptr) {
      throw Darabonba::Exception("Can't create the ProfileProvider.");
    }
    return provider_->getProviderName();
  }

protected:
  static std::unique_ptr<Provider> createProvider();

  mutable std::unique_ptr<Provider> provider_ = nullptr;
};

} // namespace Credential
} // namespace AlibabaCloud

#endif