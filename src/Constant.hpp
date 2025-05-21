#ifndef ALIBABACLOUD_CREDENTIAL_CONSTANT_HPP_
#define ALIBABACLOUD_CREDENTIAL_CONSTANT_HPP_
#include <string>
namespace Alibabacloud {
namespace Credential {
class Constant {
public:
  // Ini
  static const std::string INI_ACCESS_KEY_ID;
  static const std::string INI_ACCESS_KEY_SECRET;
  static const std::string INI_TYPE;
  static const std::string INI_TYPE_RAM;
  static const std::string INI_TYPE_ARN;
  static const std::string INI_TYPE_OIDC;
  static const std::string INI_TYPE_KEY_PAIR;
  static const std::string INI_PUBLIC_KEY_ID;
  static const std::string INI_PRIVATE_KEY_FILE;
  static const std::string INI_PRIVATE_KEY;
  static const std::string INI_ROLE_NAME;
  static const std::string INI_ROLE_SESSION_NAME;
  static const std::string INI_ROLE_ARN;
  static const std::string INI_POLICY;
  static const std::string INI_OIDC_PROVIDER_ARN;
  static const std::string INI_OIDC_TOKEN_FILE_PATH;
  static const std::string INI_ENABLE;

  // Type
  static const std::string ACCESS_KEY;
  static const std::string STS;
  static const std::string ECS_RAM_ROLE;
  static const std::string RAM_ROLE_ARN;
  static const std::string RSA_KEY_PAIR;
  static const std::string BEARER;
  static const std::string OIDC_ROLE_ARN;
  static const std::string URL_STS;
};
} // namespace Credential
} // namespace Alibabacloud

#endif