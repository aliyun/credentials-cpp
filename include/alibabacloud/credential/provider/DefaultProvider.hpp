#ifndef ALIBABACLOUD_CREDENTIAL_DEFAULTPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_DEFAULTPROVIDER_HPP_

#include <memory>
#include <string>

#include <darabonba/Exception.hpp>

#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/provider/Provider.hpp>

namespace AlibabaCloud {
namespace Credential {
class DefaultProvider : public Provider {
public:
  DefaultProvider();
  explicit DefaultProvider(std::shared_ptr<Models::Config> config);

  virtual ~DefaultProvider() = default;

  virtual Models::CredentialModel &getCredential() override {
    // If reuse enabled and we have a cached successful provider
    if (reuseLastProviderEnabled_ && lastSuccessfulProvider_) {
      try {
        return lastSuccessfulProvider_->getCredential();
      } catch (Darabonba::Exception&) {
        // Cache failed, continue trying all providers
        lastSuccessfulProvider_ = nullptr;
      }
    }

    for (auto &provider : providers_) {
      if (provider) {
        try {
          auto& credential = provider->getCredential();
          if (reuseLastProviderEnabled_) {
            lastSuccessfulProvider_ = provider.get();
          }
          return credential;
        } catch (Darabonba::Exception e) {
          continue;
        }
      }
    }
    throw Darabonba::Exception("Can't get the credential.");
  }
  
  virtual const Models::CredentialModel &getCredential() const override {
    // If reuse enabled and we have a cached successful provider
    if (reuseLastProviderEnabled_ && lastSuccessfulProvider_) {
      try {
        return lastSuccessfulProvider_->getCredential();
      } catch (Darabonba::Exception&) {
        // Cache failed, continue trying all providers
        lastSuccessfulProvider_ = nullptr;
      }
    }

    for (auto &provider : providers_) {
      if (provider) {
        try {
          auto& credential = provider->getCredential();
          if (reuseLastProviderEnabled_) {
            lastSuccessfulProvider_ = provider.get();
          }
          return credential;
        } catch (Darabonba::Exception e) {
          continue;
        }
      }
    }
    throw Darabonba::Exception("Can't get the credential.");
  }
  
  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override {
    // If reuse enabled and we have a cached successful provider
    if (reuseLastProviderEnabled_ && lastSuccessfulProvider_) {
      try {
        return lastSuccessfulProvider_->getProviderName();
      } catch (Darabonba::Exception&) {
        // Cache failed, continue trying all providers
        lastSuccessfulProvider_ = nullptr;
      }
    }

    for (auto &provider : providers_) {
      if (provider) {
        try {
          auto name = provider->getProviderName();
          if (reuseLastProviderEnabled_) {
            lastSuccessfulProvider_ = provider.get();
          }
          return name;
        } catch (Darabonba::Exception e) {
          continue;
        }
      }
    }
    throw Darabonba::Exception("Can't get the provider name.");
  }

protected:
  std::vector<std::unique_ptr<Provider>> providers_;
  bool reuseLastProviderEnabled_ = false;
  mutable Provider* lastSuccessfulProvider_ = nullptr;
};
} // namespace Credential

} // namespace AlibabaCloud
#endif