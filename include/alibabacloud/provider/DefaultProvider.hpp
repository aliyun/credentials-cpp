#ifndef AlibabaCloud_CREDENTIAL_DEFAULTPROVIDER_HPP_
#define AlibabaCloud_CREDENTIAL_DEFAULTPROVIDER_HPP_

#include <alibabacloud/Credential.hpp>
#include <alibabacloud/provider/Provider.hpp>
#include <darabonba/Env.hpp>
#include <memory>
#include <string>

namespace AlibabaCloud {
namespace Credential {
class DefaultProvider : public Provider {
public:
  DefaultProvider();
  virtual ~DefaultProvider() {}

  virtual Models::Credential &getCredential() override {
    for (auto &provider : providers_) {
      if (provider) {
        try {
          return provider->getCredential();
        } catch (Darabonba::Exception e) {
          continue;
        }
      }
    }
    throw Darabonba::Exception("Can't get the credential.");
  }
  virtual const Models::Credential &getCredential() const override {
    for (auto &provider : providers_) {
      if (provider) {
        try {
          return provider->getCredential();
        } catch (Darabonba::Exception e) {
          continue;
        }
      }
    }
    throw Darabonba::Exception("Can't get the credential.");
  }

protected:
  std::vector<std::unique_ptr<Provider>> providers_;
};
} // namespace Credential

} // namespace AlibabaCloud
#endif