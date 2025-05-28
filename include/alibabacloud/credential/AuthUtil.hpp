#ifndef AlibabaCloud_CREDENTIAL_AUTHUTIL_HPP_
#define AlibabaCloud_CREDENTIAL_AUTHUTIL_HPP_

#include <atomic>
#include <string>
#include <atomic>
#include <memory>
// #include <darabonba/lock/SpinLock.hpp>

namespace AlibabaCloud {

namespace Credential {

class AuthUtil {
public:
  static bool setClientType(const std::string &type);
  static std::string clientType();
protected:
  static std::string clientType_;
  // static Darabonba::Lock::SpinLock lock_;
};

} // namespace Credential
} // namespace AlibabaCloud

#endif