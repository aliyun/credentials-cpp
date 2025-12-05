#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/AuthUtil.hpp>

namespace AlibabaCloud {
namespace Credential {
namespace Models {

std::string Config::roleSessionName() const {
  if (roleSessionName_ != nullptr) {
    return *roleSessionName_;
  }
  // Dynamic default: credentials-cpp-{timestamp}
  return AuthUtil::generateSessionName();
}

} // namespace Models
} // namespace Credential
} // namespace AlibabaCloud
