#include <alibabacloud/credential/Constant.hpp>
namespace AlibabaCloud {
namespace Credential {

const std::string Constant::INI_ACCESS_KEY_ID = "access_key_id";
const std::string Constant::INI_ACCESS_KEY_SECRET = "access_key_secret";
const std::string Constant::INI_TYPE = "type";
const std::string Constant::INI_PUBLIC_KEY_ID = "public_key_id";
const std::string Constant::INI_PRIVATE_KEY_FILE = "private_key_file";
const std::string Constant::INI_PRIVATE_KEY = "private_key";
const std::string Constant::INI_ROLE_NAME = "role_name";
const std::string Constant::INI_ROLE_SESSION_NAME = "role_session_name";
const std::string Constant::INI_ROLE_ARN = "role_arn";
const std::string Constant::INI_POLICY = "policy";
const std::string Constant::INI_OIDC_PROVIDER_ARN = "oidc_provider_arn";
const std::string Constant::INI_OIDC_TOKEN_FILE_PATH = "oidc_token_file_path";
const std::string Constant::INI_ENABLE = "enable";

const std::string Constant::ACCESS_KEY = "access_key";
const std::string Constant::STS = "sts";
const std::string Constant::ECS_RAM_ROLE = "ecs_ram_role";
const std::string Constant::RAM_ROLE_ARN = "ram_role_arn";
const std::string Constant::RSA_KEY_PAIR = "rsa_key_pair";
const std::string Constant::BEARER = "bearer";
const std::string Constant::OIDC_ROLE_ARN = "oidc_role_arn";
const std::string Constant::URL_STS = "credentials_uri";
const std::string Constant::CLOUD_SSO = "sso";
const std::string Constant::OAUTH = "oauth";

const std::string Constant::ENV_STS_REGION = "ALIBABA_CLOUD_STS_REGION";
const std::string Constant::ENV_VPC_ENDPOINT_ENABLED =
    "ALIBABA_CLOUD_VPC_ENDPOINT_ENABLED";
const std::string Constant::ENV_CLI_PROFILE_DISABLED =
    "ALIBABA_CLOUD_CLI_PROFILE_DISABLED";

// OIDC Environment Variables
const std::string Constant::ENV_ROLE_ARN = "ALIBABA_CLOUD_ROLE_ARN";
const std::string Constant::ENV_OIDC_PROVIDER_ARN =
    "ALIBABA_CLOUD_OIDC_PROVIDER_ARN";
const std::string Constant::ENV_OIDC_TOKEN_FILE =
    "ALIBABA_CLOUD_OIDC_TOKEN_FILE";
const std::string Constant::ENV_ROLE_SESSION_NAME =
    "ALIBABA_CLOUD_ROLE_SESSION_NAME";

// Cloud SSO Environment Variables
const std::string Constant::ENV_CLOUD_SSO_ROLE_NAME = "ALIBABA_CLOUD_ROLE_NAME";

// OAuth Environment Variables
const std::string Constant::ENV_OAUTH_CLIENT_ID = "ALIBABA_CLOUD_CLIENT_ID";
const std::string Constant::ENV_OAUTH_CLIENT_SECRET =
    "ALIBABA_CLOUD_CLIENT_SECRET";
const std::string Constant::ENV_OAUTH_TOKEN_ENDPOINT =
    "ALIBABA_CLOUD_OAUTH_TOKEN_ENDPOINT";
} // namespace Credential
} // namespace AlibabaCloud
