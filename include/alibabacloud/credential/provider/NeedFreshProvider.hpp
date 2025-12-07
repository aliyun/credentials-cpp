#ifndef ALIBABACLOUD_CREDENTIAL_NEEDFRESHPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_NEEDFRESHPROVIDER_HPP_

#include <ctime>
#include <sstream>
#include <iomanip>

#include <alibabacloud/credential/provider/Provider.hpp>
namespace AlibabaCloud {
namespace Credential {
class NeedFreshProvider : public Provider {
public:
  NeedFreshProvider() = default;
  NeedFreshProvider(long long expiration) : expiration_(expiration) {}
  virtual ~NeedFreshProvider() {}

  virtual Models::CredentialModel &getCredential() override {
    refresh();
    return credential_;
  }
   virtual const Models::CredentialModel &getCredential() const override {
     refresh();
     return credential_;
   }

protected:
  virtual bool needFresh() const {
    auto now = static_cast<decltype(expiration_)>(time(nullptr));
    return expiration_ - now <= 180;
  }

  virtual bool refreshCredential() const = 0;

  virtual void refresh() const {
    if (needFresh()) {
      refreshCredential();
    }
  }

  static int64_t strtotime(const std::string &gmt) {
#ifndef _WIN32
    tm tm{};
    if (strptime(gmt.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm) == nullptr) {
      return 0; // parse failed
    }
    time_t t = timegm(&tm);
    return static_cast<int64_t>(t);
#else
    std::tm tm{};
    std::istringstream ss(gmt);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    if (ss.fail()) {
      return 0; // parse failed
    }
    time_t t = _mkgmtime(&tm);
    return static_cast<int64_t>(t);
#endif
  }

  static std::string gmt_datetime() {
    time_t now;
    time(&now);
#ifndef _WIN32
    char buf[21];
    if (strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now)) == 0) {
      return std::string();
    }
    return buf;
#else
    std::tm tm{};
    if (gmtime_s(&tm, &now) != 0) {
      return std::string();
    }
    char buf[21];
    if (std::strftime(buf, sizeof(buf), "%FT%TZ", &tm) == 0) {
      return std::string();
    }
    return std::string(buf);
#endif
  }

  mutable Models::CredentialModel credential_;
  mutable int64_t expiration_ = 0;
};
} // namespace Credential
} // namespace AlibabaCloud

#endif