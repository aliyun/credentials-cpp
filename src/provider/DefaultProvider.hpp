#ifndef ALIBABACLOUD_CREDENTIAL_DEFAULTPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_DEFAULTPROVIDER_HPP_

#include <alibabacloud/credential/Credential.hpp>
#include <alibabacloud/credential/provider/Provider.hpp>
#include <darabonba/Env.hpp>
#include <memory>
#include <string>

namespace Alibabacloud {
namespace Credential {
class DefaultProvider : public Provider {
public:
  DefaultProvider();
  virtual ~DefaultProvider() {}

  virtual Credential &getCredential() override {
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
  virtual const Credential &getCredential() const override {
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

} // namespace Alibabacloud
#endif