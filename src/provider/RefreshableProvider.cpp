#include <alibabacloud/credential/provider/RefreshableProvider.hpp>

namespace AlibabaCloud {
namespace Credential {

// C++11 requires out-of-class definition for constexpr static members
// even if they have in-class initializers
constexpr int64_t RefreshableProvider::STALE_TIME_WINDOW;
constexpr int64_t RefreshableProvider::PREFETCH_THRESHOLD;
constexpr int64_t RefreshableProvider::REFRESH_BLOCKING_MAX_WAIT_MS;

} // namespace Credential
} // namespace AlibabaCloud
