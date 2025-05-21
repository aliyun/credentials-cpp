#include <alibabacloud/AuthUtil.hpp>
#include <darabonba/Env.hpp>
#include <mutex>

namespace AlibabaCloud {
namespace Credential {

std::string AuthUtil::clientType_ = Darabonba::Env::getEnv("ALIBABA_CLOUD_PROFILE", "default");

// Darabonba::Lock::SpinLock AuthUtil::lock_;

bool AuthUtil::setClientType(const std::string &clientType) {
  // std::lock_guard<Darabonba::Lock::SpinLock> guard(lock_);
  clientType_ = clientType;
  return true;
}

std::string AuthUtil::clientType() {
  // std::lock_guard<Darabonba::Lock::SpinLock> guard(lock_);
  return clientType_;
}
}
} // namespace AlibabaCloud