# Alibaba Cloud Credentials C++ SDK - Frequently Asked Questions (FAQ)

English | [简体中文](FAQ-zh-CN.md)

This document provides solutions to common issues when using the Alibaba Cloud Credentials C++ SDK.

## Table of Contents

- [Environment Variable Issues](#environment-variable-issues)
- [Configuration File Issues](#configuration-file-issues)
- [ECS RAM Role Issues](#ecs-ram-role-issues)
- [Credential Provider Chain Issues](#credential-provider-chain-issues)
- [JSON Configuration Issues](#json-configuration-issues)
- [OIDC Credential Issues](#oidc-credential-issues)
- [Network and Timeout Issues](#network-and-timeout-issues)
- [Build and Dependency Issues](#build-and-dependency-issues)

---

## Environment Variable Issues

### Issue 1: Environment Variable accessKeyId or accessKeySecret is Empty

**Error Message:**
```
Environment variable accessKeyId cannot be empty
Environment variable accessKeySecret cannot be empty
```

**Cause:**
When using the EnvironmentVariableProvider, both `ALIBABA_CLOUD_ACCESS_KEY_ID` and `ALIBABA_CLOUD_ACCESS_KEY_SECRET` environment variables must be set.

**Solution:**
```bash
# Linux/macOS
export ALIBABA_CLOUD_ACCESS_KEY_ID="your-access-key-id"
export ALIBABA_CLOUD_ACCESS_KEY_SECRET="your-access-key-secret"

# Windows (PowerShell)
$env:ALIBABA_CLOUD_ACCESS_KEY_ID="your-access-key-id"
$env:ALIBABA_CLOUD_ACCESS_KEY_SECRET="your-access-key-secret"

# Windows (CMD)
set ALIBABA_CLOUD_ACCESS_KEY_ID=your-access-key-id
set ALIBABA_CLOUD_ACCESS_KEY_SECRET=your-access-key-secret
```

### Issue 2: IMDS Credentials Service is Disabled

**Error Message:**
```
IMDS credentials is disabled
```

**Cause:**
The environment variable `ALIBABA_CLOUD_ECS_METADATA_DISABLED` is set to `true`, preventing access to the ECS instance metadata service.

**Solution:**
1. If you need to use ECS RAM role credentials, remove or set this environment variable to `false`:
   ```bash
   # Linux/macOS
   unset ALIBABA_CLOUD_ECS_METADATA_DISABLED
   # or
   export ALIBABA_CLOUD_ECS_METADATA_DISABLED="false"
   
   # Windows (PowerShell)
   Remove-Item Env:ALIBABA_CLOUD_ECS_METADATA_DISABLED
   ```

2. If running outside of ECS environment, use alternative credential types (AccessKey or configuration file).

### Issue 3: Environment Variable Priority Confusion

**Problem:**
Multiple credential methods are configured, but it's unclear which one will be used.

**Explanation:**
The default credential provider chain searches for credentials in the following order:

1. **Environment Variables**: `ALIBABA_CLOUD_ACCESS_KEY_ID` and `ALIBABA_CLOUD_ACCESS_KEY_SECRET`
2. **OIDC RAM Role**: Configured via environment variables
3. **Configuration File**: `~/.alibabacloud/credentials.ini` or `~/.alibabacloud/credentials.json`
4. **ECS Instance RAM Role**: Retrieved via ECS metadata service
5. **Credentials URI**: Specified by `ALIBABA_CLOUD_CREDENTIALS_URI` environment variable

**Recommendations:**
- Use environment variables or configuration files in development environments
- Use ECS RAM roles or OIDC in production environments
- Avoid configuring multiple credential methods simultaneously to prevent confusion

---

## Configuration File Issues

### Issue 4: Credential Configuration File Not Found

**Error Message:**
```
No credential profile.
Can't open credential profile: /path/to/file
```

**Cause:**
- Configuration file doesn't exist or path is incorrect
- `ALIBABA_CLOUD_CREDENTIALS_FILE` environment variable is not configured
- Default paths `~/.alibabacloud/credentials.ini` or `~/.alibabacloud/credentials.json` don't exist

**Solution:**

1. Create configuration file `~/.alibabacloud/credentials.ini`:
   ```ini
   [default]
   type = access_key
   access_key_id = your-access-key-id
   access_key_secret = your-access-key-secret
   enable = true
   ```

2. Or specify configuration file path via environment variable:
   ```bash
   export ALIBABA_CLOUD_CREDENTIALS_FILE="/path/to/your/credentials.ini"
   ```

3. Ensure correct file path and read permissions:
   ```bash
   chmod 600 ~/.alibabacloud/credentials.ini
   ```

### Issue 5: Profile in Configuration File Not Enabled

**Error Message:**
```
The enable option in [profile_name] is not equal to true.
```

**Cause:**
The `enable` option in the configuration file is not set to `true` or is missing.

**Solution:**
Add or modify the `enable` option in the corresponding section:
```ini
[default]
enable = true
type = access_key
access_key_id = your-access-key-id
access_key_secret = your-access-key-secret
```

### Issue 6: Configured Credential Type is Empty

**Error Message:**
```
The configured client type is empty
```

**Cause:**
The configuration file doesn't specify the `type` field.

**Solution:**
Explicitly specify the credential type in the configuration file:
```ini
[default]
type = access_key  # Options: access_key, sts, ecs_ram_role, ram_role_arn, rsa_key_pair, oidc_role_arn
enable = true
access_key_id = your-access-key-id
access_key_secret = your-access-key-secret
```

Supported credential types:
- `access_key`: Access Key
- `sts`: Temporary Security Credentials
- `ecs_ram_role`: ECS RAM Role
- `ram_role_arn`: RAM Role ARN
- `rsa_key_pair`: RSA Key Pair (Japan site only)
- `oidc_role_arn`: OIDC Role
- `bearer`: Bearer Token
- `sso`: Cloud SSO
- `oauth`: OAuth 2.0

---

## ECS RAM Role Issues

### Issue 7: Failed to Get Credentials from ECS Metadata Service

**Error Message:**
```
Failed to get RAM session credentials from ECS metadata service. HttpCode=404
Failed to get RAM session credentials from ECS metadata service. Connection failed
```

**Cause:**
- Code is not running on an ECS instance
- ECS instance doesn't have a RAM role attached
- Network configuration blocks access to metadata service (100.100.100.200)
- IMDSv2 mode is incorrectly configured

**Solution:**

1. **Verify running on ECS instance:**
   ```bash
   curl http://100.100.100.200/latest/meta-data/
   ```
   If it returns 404 or connection failure, you're not in an ECS environment.

2. **Check if ECS instance has RAM role attached:**
   - Log into Alibaba Cloud Console
   - Navigate to ECS instance details
   - Check "Instance RAM Role" configuration

3. **Configure IMDSv1/IMDSv2:**
   ```cpp
   auto config = std::make_shared<Models::Config>();
   config->setRoleName("your-role-name")
         .setDisableIMDSv1(false);  // Allow fallback to IMDSv1
   
   EcsRamRoleProvider provider(config);
   ```

4. **Control IMDSv1 via environment variable:**
   ```bash
   export ALIBABA_CLOUD_IMDSV1_DISABLED="false"
   ```

### Issue 8: IMDSv1 Disabled Causing Credential Retrieval Failure

**Error Message:**
```
Failed to get token from ECS Metadata Service.
```

**Cause:**
When `disableIMDSv1` is set to `true`, the SDK won't fall back to IMDSv1 if IMDSv2 request fails.

**Solution:**

1. **Allow IMDSv1 fallback (recommended for development):**
   ```cpp
   auto config = std::make_shared<Models::Config>();
   config->setDisableIMDSv1(false);  // Default value
   ```

2. **Ensure ECS instance supports IMDSv2:**
   - Check ECS instance metadata service configuration
   - Enable IMDSv2 support in Alibaba Cloud Console

3. **Adjust timeout settings:**
   ```cpp
   auto config = std::make_shared<Models::Config>();
   config->setTimeout(5000)          // Read timeout (milliseconds)
         .setConnectTimeout(10000);  // Connect timeout (milliseconds)
   ```

---

## Credential Provider Chain Issues

### Issue 9: All Providers in Chain Failed

**Error Message:**
Depends on the last provider's error message.

**Cause:**
The default credential provider chain tries each provider sequentially. If all fail, it throws the last error.

**Solution:**

1. **Check each credential source:**
   - Are environment variables correctly set?
   - Does the configuration file exist with correct format?
   - Does the ECS instance have a RAM role attached?

2. **Use explicit credential type:**
   ```cpp
   // Don't use default chain, explicitly specify credential type
   Models::Config config;
   config.setType(Constant::ACCESS_KEY)
         .setAccessKeyId("your-ak")
         .setAccessKeySecret("your-secret");
   
   Client client(config);
   ```

3. **Enable debug logging:**
   View specific failure reasons for each provider to identify the issue.

### Issue 10: Empty Credential Chain

**Error Message:**
```
No provider available in credential chain
```

**Cause:**
The created ChainProvider contains no providers.

**Solution:**
Ensure the credential chain has at least one provider:
```cpp
std::vector<std::shared_ptr<CredentialProvider>> providers;
providers.push_back(std::make_shared<AccessKeyProvider>("ak", "secret"));
providers.push_back(std::make_shared<EcsRamRoleProvider>());

ChainProvider provider(providers);
```

---

## JSON Configuration Issues

### Issue 11: JSON Format Configuration Not Supported

**Error Message:**
```
JSON format is not supported. Please install nlohmann_json library or use INI format.
```

**Cause:**
SDK was compiled without JSON support (`HAS_NLOHMANN_JSON` macro not defined).

**Solution:**

1. **Install nlohmann_json library:**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install nlohmann-json3-dev
   
   # macOS
   brew install nlohmann-json
   
   # Windows (vcpkg)
   vcpkg install nlohmann-json
   ```

2. **Rebuild the SDK:**
   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```

3. **Or use INI format configuration:**
   ```ini
   [default]
   type = access_key
   access_key_id = your-access-key-id
   access_key_secret = your-access-key-secret
   enable = true
   ```

### Issue 12: JSON Configuration File Format Error

**Error Message:**
```
Failed to parse JSON profile: [parse error details]
No 'profiles' section in CLI config file
'profiles' must be an array
```

**Cause:**
JSON configuration file format is incorrect.

**Solution:**
Use the correct JSON format (following Alibaba Cloud CLI configuration):
```json
{
  "profiles": [
    {
      "name": "default",
      "mode": "AK",
      "access_key_id": "your-access-key-id",
      "access_key_secret": "your-access-key-secret"
    }
  ]
}
```

### Issue 13: Profile Not Found in CLI Configuration File

**Error Message:**
```
Profile 'profile_name' not found in CLI config
```

**Cause:**
The specified profile name doesn't exist in the JSON configuration file.

**Solution:**

1. **Check profile name correctness:**
   ```cpp
   CLIProfileProvider provider("default");  // Ensure name matches
   ```

2. **Add profile to configuration file:**
   ```json
   {
     "profiles": [
       {
         "name": "default",
         "mode": "AK",
         "access_key_id": "your-ak",
         "access_key_secret": "your-secret"
       },
       {
         "name": "production",
         "mode": "AK",
         "access_key_id": "prod-ak",
         "access_key_secret": "prod-secret"
       }
     ]
   }
   ```

3. **Use environment variable to specify profile:**
   ```bash
   export ALIBABA_CLOUD_PROFILE="production"
   ```

---

## OIDC Credential Issues

### Issue 14: Cannot Open OIDC Token File

**Error Message:**
```
Can't open /path/to/oidc/token/file
```

**Cause:**
- OIDC Token file path doesn't exist
- Insufficient file permissions
- Environment variable `ALIBABA_CLOUD_OIDC_TOKEN_FILE` is incorrectly configured

**Solution:**

1. **Check if file exists:**
   ```bash
   ls -l /path/to/oidc/token/file
   ```

2. **Ensure file is readable:**
   ```bash
   chmod 644 /path/to/oidc/token/file
   ```

3. **Correctly configure environment variables:**
   ```bash
   export ALIBABA_CLOUD_ROLE_ARN="acs:ram::account-id:role/role-name"
   export ALIBABA_CLOUD_OIDC_PROVIDER_ARN="acs:ram::account-id:oidc-provider/provider-name"
   export ALIBABA_CLOUD_OIDC_TOKEN_FILE="/var/run/secrets/token"
   ```

4. **Specify in configuration file:**
   ```ini
   [default]
   type = oidc_role_arn
   role_arn = acs:ram::account-id:role/role-name
   oidc_provider_arn = acs:ram::account-id:oidc-provider/provider-name
   oidc_token_file_path = /var/run/secrets/token
   enable = true
   ```

### Issue 15: OIDC Credential Request Failed

**Error Message:**
```
Failed to get credentials via OIDC
```

**Cause:**
- Token file content is invalid or expired
- OIDC Provider ARN or Role ARN is incorrectly configured
- Network connection issues

**Solution:**

1. **Validate token content:**
   ```bash
   cat /var/run/secrets/token
   # Should contain a valid JWT Token
   ```

2. **Check ARN configuration:**
   - Log into Alibaba Cloud RAM Console
   - Verify OIDC provider and role configuration
   - Ensure role's trust policy includes the OIDC provider

3. **Check network connectivity:**
   ```bash
   curl -I https://sts.aliyuncs.com
   ```

---

## Network and Timeout Issues

### Issue 16: Request Timeout

**Error Message:**
```
Request timeout
Connection timeout
```

**Cause:**
- High network latency
- Default timeout too short
- Firewall or proxy configuration blocking access

**Solution:**

1. **Increase timeout:**
   ```cpp
   auto config = std::make_shared<Models::Config>();
   config->setTimeout(10000)          // Read timeout: 10 seconds
         .setConnectTimeout(15000);   // Connect timeout: 15 seconds
   
   Provider* provider = new EcsRamRoleProvider(config);
   ```

2. **Configure proxy:**
   ```cpp
   config->setProxy("http://proxy.example.com:8080");
   ```

3. **Check network connectivity:**
   ```bash
   # Test ECS metadata service
   curl -v http://100.100.100.200/latest/meta-data/
   
   # Test STS service
   curl -v https://sts.aliyuncs.com
   ```

### Issue 17: CloudSSO or OAuth Credential Retrieval Failed

**Error Message:**
```
Failed to get credentials from Cloud SSO. Status code is XXX
Failed to get credentials via OAuth. Status code is XXX
```

**Cause:**
- Authentication service endpoint unreachable
- Credential configuration incorrect
- Token expired

**Solution:**

1. **CloudSSO configuration:**
   ```cpp
   Models::Config config;
   config.setType(Constant::CLOUD_SSO)
         .setRoleName("role-name")
         .setRegionId("cn-hangzhou");
   ```

2. **OAuth configuration:**
   ```cpp
   Models::Config config;
   config.setType(Constant::OAUTH)
         .setAccessKeyId("client-id")
         .setAccessKeySecret("client-secret")
         .setStsEndpoint("https://oauth.aliyuncs.com/v1/token")
         .setRegionId("cn-hangzhou");
   ```

3. **Check service endpoint:**
   ```bash
   curl -v https://oauth.aliyuncs.com/v1/token
   ```

---

## Build and Dependency Issues

### Issue 18: Missing OpenSSL Dependency

**Error Message:**
```
fatal error: openssl/ssl.h: No such file or directory
```

**Cause:**
OpenSSL development library not installed on the system.

**Solution:**

```bash
# Ubuntu/Debian
sudo apt-get install libssl-dev

# CentOS/RHEL
sudo yum install openssl-devel

# macOS
brew install openssl

# Windows (vcpkg)
vcpkg install openssl
```

### Issue 19: CMake Cannot Find Dependencies

**Error Message:**
```
Could not find a package configuration file provided by "OpenSSL"
```

**Cause:**
CMake cannot find OpenSSL or other dependency libraries.

**Solution:**

1. **Specify OpenSSL path (macOS):**
   ```bash
   cmake -S . -B build \
     -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl \
     -DCMAKE_BUILD_TYPE=Release
   ```

2. **Use vcpkg (Windows):**
   ```bash
   cmake -S . -B build \
     -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake \
     -DCMAKE_BUILD_TYPE=Release
   ```

3. **Verify dependency installation:**
   ```bash
   pkg-config --modversion openssl
   ```

### Issue 20: C++ Standard Version Incompatibility

**Error Message:**
```
error: 'auto' not allowed in lambda parameter
```

**Cause:**
Compiler doesn't support C++11 or higher standard.

**Solution:**

1. **Upgrade compiler:**
   - GCC 4.9 or higher
   - Visual Studio 2015 or higher
   - Clang (Xcode Command Line Tools)

2. **Explicitly specify C++ standard:**
   ```bash
   cmake -S . -B build -DCMAKE_CXX_STANDARD=11
   ```

---

## Best Practices

### Development Environment

1. **Use environment variables or configuration files:**
   ```bash
   export ALIBABA_CLOUD_ACCESS_KEY_ID="dev-ak"
   export ALIBABA_CLOUD_ACCESS_KEY_SECRET="dev-secret"
   ```

2. **Use separate test credentials:**
   Avoid using production AccessKeys.

3. **Enable verbose logging:**
   Helpful for troubleshooting issues.

### Production Environment

1. **Prioritize ECS RAM roles or OIDC:**
   No need to hardcode credentials in code or configuration.

2. **Set reasonable timeout values:**
   ```cpp
   config->setTimeout(5000)
         .setConnectTimeout(10000);
   ```

3. **Use automatic credential refresh:**
   ECS RAM role and OIDC credentials refresh automatically.

4. **Avoid logging credentials:**
   ```cpp
   // Bad example
   printf("AccessKeySecret: %s\n", credential.getAccessKeySecret().c_str());
   
   // Good example
   printf("Credential Type: %s\n", credential.getType().c_str());
   ```

### Security Recommendations

1. **Configuration file permissions:**
   ```bash
   chmod 600 ~/.alibabacloud/credentials.ini
   ```

2. **Regularly rotate AccessKeys:**
   Recommended every 90 days.

3. **Use least privilege principle:**
   Assign minimal necessary permissions to RAM roles and users.

4. **Don't commit credentials to code repositories:**
   Add configuration files to `.gitignore`.

---

## Getting Help

If the above solutions don't resolve your issue, please:

1. **Refer to official documentation:**
   - [GitHub Repository](https://github.com/aliyun/credentials-cpp)
   - [Alibaba Cloud OpenAPI Developer Portal](https://next.api.aliyun.com)

2. **Submit an Issue:**
   - [GitHub Issues](https://github.com/aliyun/credentials-cpp/issues/new/choose)
   - Please provide detailed error messages, environment information, and reproduction steps

3. **Reference related projects:**
   - [credentials-java](https://github.com/aliyun/credentials-java)
   - [credentials-python](https://github.com/aliyun/credentials-python)
   - [credentials-go](https://github.com/aliyun/credentials-go)

---

## Version Information

- Document Updated: 2026-01-10
- Applicable SDK Version: credentials-cpp latest
- Reference Project: [aliyun/credentials-cpp](https://github.com/aliyun/credentials-cpp)

---

## Contributing

Contributions via PR are welcome to supplement more common issues and solutions.
