#ifndef ALIBABACLOUD_ENDPOINTUTIL_H_
#define ALIBABACLOUD_ENDPOINTUTIL_H_

#include <string>

namespace Alibabacloud {
class EndpointUtil {
public:
  static std::string getEndpointRules(const std::string &product,
                                      const std::string &regionId,
                                      const std::string &endpointType,
                                      const std::string &network,
                                      const std::string &suffix);
};

} // namespace Alibabacloud

#endif