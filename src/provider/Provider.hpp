#ifndef ALIBABACLOUD_CREDENTIAL_PROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_PROVIDER_HPP_

#include <alibabacloud/credential/Credential.hpp>
#include <memory>

namespace Alibabacloud {
namespace Credential {
class Provider {
public:
  Provider() = default;
  virtual ~Provider() {}

  virtual Credential &getCredential() = 0;
  virtual const Credential &getCredential() const = 0;
};
} // namespace Credential

} // namespace Alibabacloud
#endif