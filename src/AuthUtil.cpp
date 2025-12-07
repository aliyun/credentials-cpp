#include <chrono>
#include <sstream>

#include <darabonba/Env.hpp>
#include <darabonba/http/Request.hpp>

#include <alibabacloud/credential/AuthUtil.hpp>

namespace AlibabaCloud {
namespace Credential {

std::string AuthUtil::clientType_ =
    Darabonba::Env::getEnv("ALIBABA_CLOUD_PROFILE", "default");

bool AuthUtil::setClientType(const std::string &clientType) {
  clientType_ = clientType;
  return true;
}

std::string AuthUtil::clientType() { return clientType_; }

std::string AuthUtil::generateSessionName() {
  const auto now = std::chrono::system_clock::now();
  const auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                             now.time_since_epoch())
                             .count();
  return "credentials-cpp-" + std::to_string(timestamp);
}

/**
 * @brief Get SDK version from CMake project version
 *
 * Returns version string, default "0.1.0" (from CMakeLists.txt)
 */
std::string AuthUtil::getSDKVersion() {
  // Version is defined in CMakeLists.txt: VERSION "0.1.0"
  // In production, this could be generated during build time
  return "0.1.0";
}

/**
 * @brief Get operating system name
 *
 * Platform detection based on preprocessor macros
 */
std::string AuthUtil::getOSName() {
#if defined(_WIN32) || defined(_WIN64)
  return "Windows";
#elif defined(__APPLE__) || defined(__MACH__)
  return "macOS";
#elif defined(__linux__)
  return "Linux";
#elif defined(__unix__) || defined(__unix)
  return "Unix";
#else
  return "Unknown";
#endif
}

/**
 * @brief Get machine architecture name
 *
 * Corresponds to Python's platform.machine()
 * Returns architecture like x86_64, arm64, etc.
 */
std::string AuthUtil::getMachineName() {
#if defined(__x86_64__) || defined(_M_X64)
  return "x86_64";
#elif defined(__aarch64__) || defined(_M_ARM64)
  return "arm64";
#elif defined(__i386__) || defined(_M_IX86)
  return "i386";
#elif defined(__arm__) || defined(_M_ARM)
  return "arm";
#else
  return "unknown";
#endif
}

/**
 * @brief Get C++ standard version
 *
 * Corresponds to Python's platform.python_version()
 * Returns C++ version like "11", "14", "17", "20"
 */
std::string AuthUtil::getCppVersion() {
#if __cplusplus >= 202002L
  return "20";
#elif __cplusplus >= 201703L
  return "17";
#elif __cplusplus >= 201402L
  return "14";
#elif __cplusplus >= 201103L
  return "11";
#else
  return "98";
#endif
}

/**
 * @brief Get User-Agent string
 *
 * Corresponds to Python SDK's get_user_agent() in parameter_helper.py
 * Python format:
 *   AlibabaCloud ({platform.system()}; {platform.machine()})
 *   Python/{platform.python_version()}
 *   Credentials/{alibabacloud_credentials.__version__}
 *   TeaDSL/2
 *
 * C++ format:
 *   AlibabaCloud ({os}; {machine})
 *   C++/{cpp_version}
 *   Credentials/{credentials_version}
 *   TeaDSL/2 {custom_ua}
 *
 * @param customUserAgent Custom user agent suffix (optional)
 * @return Complete User-Agent string
 */
std::string AuthUtil::getUserAgent(const std::string &customUserAgent) {
  std::ostringstream oss;

  // Format: AlibabaCloud ({os}; {machine}) C++/{version}
  // Credentials/{credentials_version} TeaDSL/2
  oss << "AlibabaCloud (" << getOSName() << "; " << getMachineName() << ") "
      << "C++/" << getCppVersion() << " "
      << "Credentials/" << getSDKVersion() << " "
      << "TeaDSL/2";

  // Append custom user agent if provided
  if (!customUserAgent.empty()) {
    oss << " " << customUserAgent;
  }

  return oss.str();
}

/**
 * @brief Create a new HTTP request with User-Agent header
 *
 * Corresponds to Python SDK's getNewRequest() helper function
 * Used by providers like EcsRamRoleProvider, RsaKeyPairProvider
 *
 * Python implementation:
 * def getNewRequest(url, custom_ua=None):
 *     req = HTTPRequest(url, method='GET')
 *     req.headers['User-Agent'] = get_user_agent(custom_ua)
 *     return req
 *
 * @param url Request URL
 * @param customUserAgent Custom user agent suffix (optional)
 * @return HTTP Request object with User-Agent header set
 */
Darabonba::Http::Request
AuthUtil::getNewRequest(const std::string &url,
                        const std::string &customUserAgent) {
  Darabonba::Http::Request req(url);

  // Set User-Agent header
  req.headers()["User-Agent"] = getUserAgent(customUserAgent);

  return req;
}

} // namespace Credential
} // namespace AlibabaCloud