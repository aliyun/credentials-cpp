#include <alibabacloud/credential/provider/RefreshableProvider.hpp>

namespace AlibabaCloud {
namespace Credential {

// Define the static constants
const std::chrono::milliseconds RefreshableProvider::REFRESH_BLOCKING_MAX_WAIT = std::chrono::milliseconds(10000);
const int64_t RefreshableProvider::STALE_TIME_WINDOW;
const int64_t RefreshableProvider::PREFETCH_THRESHOLD;

} // namespace Credential
} // namespace AlibabaCloud
