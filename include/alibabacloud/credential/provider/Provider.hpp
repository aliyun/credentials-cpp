#ifndef AlibabaCloud_CREDENTIAL_PROVIDER_HPP_
#define AlibabaCloud_CREDENTIAL_PROVIDER_HPP_

#include <alibabacloud/credential/Model.hpp>
#include <memory>


namespace AlibabaCloud {
namespace Credential {
class Provider {
public:
  Provider() = default;
  virtual ~Provider() {}

  virtual Models::CredentialModel &getCredential() = 0;
  virtual const Models::CredentialModel &getCredential() const = 0;
};
} // namespace Credential
} // namespace AlibabaCloud
#endif