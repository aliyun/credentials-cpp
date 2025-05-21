#ifndef AlibabaCloud_CREDENTIAL_NEEDFRESHPROVIDER_HPP_
#define AlibabaCloud_CREDENTIAL_NEEDFRESHPROVIDER_HPP_

#include <alibabacloud/provider/Provider.hpp>
#include <ctime>

#include <ctime>
namespace AlibabaCloud {
namespace Credential {
class NeedFreshProvider : public Provider {
public:
  NeedFreshProvider() = default;
  NeedFreshProvider(long long expiration) : expiration_(expiration) {}
  virtual ~NeedFreshProvider() {}

  virtual Models::Credential &getCredential() override {
    refresh();
    return credential_;
  }
  virtual const Models::Credential &getCredential() const override {
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
    tm tm;
    strptime(gmt.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm);
    time_t t = timegm(&tm);
    return static_cast<int64_t>(t);
  }

  static std::string gmt_datetime() {
    time_t now;
    time(&now);
    char buf[20];
    strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
    return buf;
  }

  mutable Models::Credential credential_;
  mutable int64_t expiration_ = 0;
};
} // namespace Credential
} // namespace AlibabaCloud

#endif