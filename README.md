English | [简体中文](/README-zh-CN.md)

# Alibaba Cloud Credentials for C++

[![codecov](https://codecov.io/gh/aliyun/credentials-cpp/branch/master/graph/badge.svg)](https://codecov.io/gh/aliyun/credentials-cpp)
[![Travis Build Status](https://travis-ci.org/aliyun/credentials-cpp.svg?branch=master)](https://travis-ci.org/aliyun/credentials-cpp)

![](https://aliyunsdk-pages.alicdn.com/icons/AlibabaCloud.svg)

Alibaba Cloud Credentials for C++ is a tool that helps C++ developers manage their credentials.

## Table of Contents

- [Requirements](#requirements)
- [Installation](#installation)
- [Quick Examples](#quick-examples)
- [Credential Types](#credential-types)
  - [Default Credentials Provider Chain](#default-credentials-provider-chain)
  - [AccessKey](#accesskey)
  - [STS](#sts)
  - [RamRoleArn](#ramrolearn)
  - [OIDCRoleArn](#oidcrolearn)
  - [EcsRamRole](#ecsramrole)
  - [URLCredential](#urlcredential)
  - [BearerToken](#bearertoken)
  - [RsaKeyPair](#rsakeypair)
- [Configuration](#configuration)
- [Issues](#issues)
- [Release Notes](#release-notes)
- [Related](#related)
- [License](#license)

## Requirements

### Compiler Requirements

- **Windows**: Visual Studio 2015 or later
- **Linux**: GCC 4.9 or later
- **macOS**: Clang (Xcode Command Line Tools)

### Build Tools

- **CMake**: 3.10 or later (3.15+ recommended for Windows)
- **C++ Standard**: C++11 or higher

### System Requirements

- **Memory**: 4GB or more recommended
- **Disk Space**: At least 500MB available space

### Dependencies

This library depends on **darabonba-core**, which requires the following system libraries:

- **OpenSSL**: For encryption and network communication
  - Windows: Install via vcpkg (`vcpkg install openssl`) or chocolatey (`choco install openssl`)
  - Linux: `sudo apt-get install libssl-dev` (Ubuntu/Debian) or `sudo yum install openssl-devel` (CentOS/RHEL)
  - macOS: `brew install openssl`

- **libcurl**: For HTTP requests
  - Windows: Install via vcpkg (`vcpkg install curl`)
  - Linux: `sudo apt-get install libcurl4-openssl-dev` (Ubuntu/Debian) or `sudo yum install libcurl-devel` (CentOS/RHEL)
  - macOS: Preinstalled or `brew install curl`

- **zlib**: For compression (required by curl, especially for static builds)
  - Windows: Install via vcpkg (`vcpkg install zlib`)
  - Linux: `sudo apt-get install zlib1g-dev` (Ubuntu/Debian) or `sudo yum install zlib-devel` (CentOS/RHEL)
  - macOS: Preinstalled or `brew install zlib`

- **uuid** (Linux only): For UUID generation
  - Ubuntu/Debian: `sudo apt-get install uuid-dev`
  - CentOS/RHEL: `sudo yum install libuuid-devel`
  - macOS: Not required (uses CoreFoundation framework)
  - Windows: Not required (uses Windows UUID API)

**Quick Install (All Dependencies):**

```bash
# Ubuntu/Debian
sudo apt-get install -y libssl-dev libcurl4-openssl-dev zlib1g-dev uuid-dev

# CentOS/RHEL
sudo yum install -y openssl-devel libcurl-devel zlib-devel libuuid-devel

# macOS (Homebrew)
brew install openssl curl

# Windows (vcpkg)
vcpkg install openssl curl zlib
```

## Installation

### Linux/macOS

```bash
git clone https://github.com/aliyun/credentials-cpp.git
cd credentials-cpp
sh scripts/install.sh
```

### Windows

1. Run the following command to clone code from GitHub via git-bash:

```bash
git clone https://github.com/aliyun/credentials-cpp.git
```

2. Build using Visual Studio:
   - Create a `cmake_build` directory in the root directory
   - Open CMake GUI, then:
     - Set `Browse Source` to the source code directory (`credentials-cpp`)
     - Set `Browse Build` to the build directory (`cmake_build`)
     - Click `Configure`
     - Click `Generate` to generate the VS solution

3. Build and Install C++ SDK:
   - Go to the `cmake_build` directory and open `alibabacloud_credentials.sln` with Visual Studio
   - Select `Release` configuration
   - Check INSTALL option from Build -> Configuration Manager
   - Build -> Build Solution

### Quick Build

```bash
# Clone and build
git clone https://github.com/aliyun/credentials-cpp.git
cd credentials-cpp
mkdir build && cd build
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cmake --install .
```

**📖 For detailed build instructions, see [BUILD.md](BUILD.md) | [BUILD-zh-CN.md](BUILD-zh-CN.md)**

## Quick Examples

Before you begin, you need to sign up for an Alibaba Cloud account and retrieve your [Credentials](https://usercenter.console.aliyun.com/#/manage/ak).

**📚 For more examples, see [examples/README.md](examples/README.md) | [examples/README-zh-CN.md](examples/README-zh-CN.md)**

### Using Default Credentials Provider Chain (Recommended)

```cpp
#include <alibabacloud/credentials/Client.hpp>
#include <iostream>

using namespace AlibabaCloud::Credentials;

int main() {
    // Use default credentials provider chain
    Client client;
    
    // Get credentials
    auto credential = client.getCredential();
    std::cout << "AccessKeyId: " << credential.getAccessKeyId() << std::endl;
    std::cout << "AccessKeySecret: " << credential.getAccessKeySecret() << std::endl;
    std::cout << "Type: " << credential.getType() << std::endl;
    
    return 0;
}
```

### Using Specific Credential Type

```cpp
#include <alibabacloud/credentials/Client.hpp>
#include <iostream>

using namespace AlibabaCloud::Credentials;

int main() {
    // Create config with AccessKey
    Models::Config config;
    config.setType("access_key")
          .setAccessKeyId("<your-access-key-id>")
          .setAccessKeySecret("<your-access-key-secret>");
    
    // Create client with config
    Client client(config);
    
    // Get credentials
    auto credential = client.getCredential();
    std::cout << "AccessKeyId: " << credential.getAccessKeyId() << std::endl;
    
    return 0;
}
```

## Credential Types

### Default Credentials Provider Chain

If you do not specify a credential type, the client will search for credentials in the following order:

1. **Environment Variables**: `ALIBABA_CLOUD_ACCESS_KEY_ID` and `ALIBABA_CLOUD_ACCESS_KEY_SECRET`
2. **OIDC RAM Role**: Configured via environment variables `ALIBABA_CLOUD_ROLE_ARN`, `ALIBABA_CLOUD_OIDC_PROVIDER_ARN`, and `ALIBABA_CLOUD_OIDC_TOKEN_FILE`
3. **Configuration File**: `~/.alibabacloud/credentials.ini` or `~/.alibabacloud/credentials.json`
4. **ECS Instance RAM Role**: Retrieved via ECS Instance Metadata Service (IMDS), if `ALIBABA_CLOUD_ECS_METADATA` is set
5. **Credentials URI**: Retrieved from URL specified by environment variable `ALIBABA_CLOUD_CREDENTIALS_URI`

**Setting credentials via environment variables:**

```bash
# Linux/macOS
export ALIBABA_CLOUD_ACCESS_KEY_ID="<your-access-key-id>"
export ALIBABA_CLOUD_ACCESS_KEY_SECRET="<your-access-key-secret>"

# Windows (PowerShell)
$env:ALIBABA_CLOUD_ACCESS_KEY_ID="<your-access-key-id>"
$env:ALIBABA_CLOUD_ACCESS_KEY_SECRET="<your-access-key-secret>"

# Windows (CMD)
set ALIBABA_CLOUD_ACCESS_KEY_ID=<your-access-key-id>
set ALIBABA_CLOUD_ACCESS_KEY_SECRET=<your-access-key-secret>
```

**Setting credentials via configuration file (`~/.alibabacloud/credentials.ini`):**

```ini
[default]
type = access_key
access_key_id = <your-access-key-id>
access_key_secret = <your-access-key-secret>
```

### AccessKey

Setup access_key credential through [User Information Management][ak]. It has full authority over the account, please keep it safe. Sometimes for security reasons, you cannot hand over a primary account AccessKey with full access to the developer of a project. You may create a sub-account [RAM Sub-account][ram], grant its [authorization][permissions], and use the AccessKey of RAM Sub-account.

```cpp
#include <alibabacloud/credentials/Client.hpp>

using namespace AlibabaCloud::Credentials;

int main() {
    Models::Config config;
    config.setType("access_key")
          .setAccessKeyId("<your-access-key-id>")
          .setAccessKeySecret("<your-access-key-secret>");
    
    Client client(config);
    
    auto credential = client.getCredential();
    printf("AccessKeyId: %s\n", credential.getAccessKeyId().c_str());
    printf("AccessKeySecret: %s\n", credential.getAccessKeySecret().c_str());
    
    return 0;
}
```

### STS

Create a temporary security credential by applying Temporary Security Credentials (TSC) through the Security Token Service (STS).

```cpp
#include <alibabacloud/credentials/Client.hpp>

using namespace AlibabaCloud::Credentials;

int main() {
    Models::Config config;
    config.setType("sts")
          .setAccessKeyId("<your-access-key-id>")
          .setAccessKeySecret("<your-access-key-secret>")
          .setSecurityToken("<your-security-token>");
    
    Client client(config);
    
    auto credential = client.getCredential();
    printf("AccessKeyId: %s\n", credential.getAccessKeyId().c_str());
    printf("AccessKeySecret: %s\n", credential.getAccessKeySecret().c_str());
    printf("SecurityToken: %s\n", credential.getSecurityToken().c_str());
    
    return 0;
}
```

### RamRoleArn

By specifying [RAM Role][RAM Role], the credential will be able to automatically request maintenance of STS Token. If you want to limit the permissions ([How to make a policy][policy]) of STS Token, you can assign value for `Policy`.

```cpp
#include <alibabacloud/credentials/Client.hpp>

using namespace AlibabaCloud::Credentials;

int main() {
    Models::Config config;
    config.setType("ram_role_arn")
          .setAccessKeyId("<your-access-key-id>")
          .setAccessKeySecret("<your-access-key-secret>")
          .setRoleArn("<your-role-arn>")
          .setRoleSessionName("<your-role-session-name>")
          // Optional: set policy, duration, and region
          .setPolicy("<your-policy>")
          .setDurationSeconds(3600)
          .setStsRegionId("cn-hangzhou");
    
    Client client(config);
    
    auto credential = client.getCredential();
    printf("AccessKeyId: %s\n", credential.getAccessKeyId().c_str());
    printf("AccessKeySecret: %s\n", credential.getAccessKeySecret().c_str());
    printf("SecurityToken: %s\n", credential.getSecurityToken().c_str());
    
    return 0;
}
```

### OIDCRoleArn

By specifying OIDC Role, the credential will be able to automatically request maintenance of STS Token using OIDC Token.

```cpp
#include <alibabacloud/credentials/Client.hpp>

using namespace AlibabaCloud::Credentials;

int main() {
    Models::Config config;
    config.setType("oidc_role_arn")
          .setRoleArn("<your-role-arn>")
          .setOidcProviderArn("<your-oidc-provider-arn>")
          .setOidcTokenFilePath("<path-to-oidc-token-file>")
          .setRoleSessionName("<your-role-session-name>")
          // Optional: set policy, duration, and region
          .setDurationSeconds(3600)
          .setStsRegionId("cn-hangzhou");
    
    Client client(config);
    
    auto credential = client.getCredential();
    printf("AccessKeyId: %s\n", credential.getAccessKeyId().c_str());
    printf("AccessKeySecret: %s\n", credential.getAccessKeySecret().c_str());
    printf("SecurityToken: %s\n", credential.getSecurityToken().c_str());
    
    return 0;
}
```

**Or configure via environment variables:**

```bash
export ALIBABA_CLOUD_ROLE_ARN="<your-role-arn>"
export ALIBABA_CLOUD_OIDC_PROVIDER_ARN="<your-oidc-provider-arn>"
export ALIBABA_CLOUD_OIDC_TOKEN_FILE="<path-to-oidc-token-file>"
export ALIBABA_CLOUD_ROLE_SESSION_NAME="<your-role-session-name>"
```

### EcsRamRole

By specifying the role name, the credential will be able to automatically request maintenance of STS Token.

By default, the Credentials tool accesses the metadata server of ECS in security hardening mode (IMDSv2). If an exception is thrown, the Credentials tool switches to the normal mode (IMDSv1). You can configure the `disableIMDSv1` parameter or the `ALIBABA_CLOUD_IMDSV1_DISABLE` environment variable to specify the exception handling logic:

- `false` (default): The Credentials tool continues to obtain the access credential in normal mode (IMDSv1).
- `true`: The exception is thrown and the Credentials tool continues to obtain the access credential in security hardening mode.

You can specify `ALIBABA_CLOUD_ECS_METADATA_DISABLED=true` to disable access from the Credentials tool to the metadata server of ECS.

```cpp
#include <alibabacloud/credentials/Client.hpp>

using namespace AlibabaCloud::Credentials;

int main() {
    Models::Config config;
    config.setType("ecs_ram_role")
          .setRoleName("<your-ecs-role-name>")
          // Optional: disable IMDSv1 for enhanced security
          .setDisableIMDSv1(false);
    
    Client client(config);
    
    auto credential = client.getCredential();
    printf("AccessKeyId: %s\n", credential.getAccessKeyId().c_str());
    printf("AccessKeySecret: %s\n", credential.getAccessKeySecret().c_str());
    printf("SecurityToken: %s\n", credential.getSecurityToken().c_str());
    
    return 0;
}
```

**Or configure via environment variable:**

```bash
export ALIBABA_CLOUD_ECS_METADATA="<your-ecs-role-name>"
```

### URLCredential

Get credentials by specifying a URL and the credential tool will automatically request this URL to obtain credentials.

```cpp
#include <alibabacloud/credentials/Client.hpp>

using namespace AlibabaCloud::Credentials;

int main() {
    Models::Config config;
    config.setType("credentials_uri")
          .setCredentialsUri("<your-credentials-url>");
    
    Client client(config);
    
    auto credential = client.getCredential();
    printf("AccessKeyId: %s\n", credential.getAccessKeyId().c_str());
    printf("AccessKeySecret: %s\n", credential.getAccessKeySecret().c_str());
    printf("SecurityToken: %s\n", credential.getSecurityToken().c_str());
    
    return 0;
}
```

**Or configure via environment variable:**

```bash
export ALIBABA_CLOUD_CREDENTIALS_URI="<your-credentials-url>"
```

The response from the URL must be in the following format:

```json
{
  "Code": "Success",
  "AccessKeyId": "<your-access-key-id>",
  "AccessKeySecret": "<your-access-key-secret>",
  "SecurityToken": "<your-security-token>",
  "Expiration": "2023-12-31T12:00:00Z"
}
```

### BearerToken

If credential is required by the Cloud Call Centre (CCC), please apply for Bearer Token maintenance by yourself.

```cpp
#include <alibabacloud/credentials/Client.hpp>

using namespace AlibabaCloud::Credentials;

int main() {
    Models::Config config;
    config.setType("bearer")
          .setBearerToken("<your-bearer-token>");
    
    Client client(config);
    
    auto credential = client.getCredential();
    printf("BearerToken: %s\n", credential.getBearerToken().c_str());
    
    return 0;
}
```

### RsaKeyPair

By specifying the public key ID and the private key file, the credential will be able to automatically request maintenance of the AccessKey. Only Japan station is supported.

```cpp
#include <alibabacloud/credentials/Client.hpp>

using namespace AlibabaCloud::Credentials;

int main() {
    Models::Config config;
    config.setType("rsa_key_pair")
          .setPublicKeyId("<your-public-key-id>")
          .setPrivateKeyFile("<path-to-private-key-file>");
    
    Client client(config);
    
    auto credential = client.getCredential();
    printf("AccessKeyId: %s\n", credential.getAccessKeyId().c_str());
    printf("AccessKeySecret: %s\n", credential.getAccessKeySecret().c_str());
    
    return 0;
}
```

## Configuration

### Config Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | string | Credential type |
| `accessKeyId` | string | Access Key ID |
| `accessKeySecret` | string | Access Key Secret |
| `securityToken` | string | Security Token |
| `bearerToken` | string | Bearer Token |
| `roleArn` | string | RAM Role ARN |
| `roleSessionName` | string | Role session name |
| `policy` | string | Permission policy (JSON string) |
| `durationSeconds` | int64_t | Session duration in seconds (default: 3600) |
| `roleName` | string | ECS RAM role name |
| `oidcProviderArn` | string | OIDC Provider ARN |
| `oidcTokenFilePath` | string | Path to OIDC token file |
| `publicKeyId` | string | RSA public key ID |
| `privateKeyFile` | string | Path to RSA private key file |
| `credentialsUri` | string | URL to fetch credentials |
| `stsEndpoint` | string | STS endpoint (default: sts.aliyuncs.com) |
| `stsRegionId` | string | STS region ID |
| `regionId` | string | Region ID (default: cn-hangzhou) |
| `timeout` | int64_t | Read timeout in milliseconds (default: 5000) |
| `connectTimeout` | int64_t | Connection timeout in milliseconds (default: 10000) |
| `disableIMDSv1` | bool | Disable IMDSv1 for ECS metadata (default: false) |
| `enableVpc` | bool | Enable VPC endpoint (default: false) |

### Environment Variables

| Variable | Description |
|----------|-------------|
| `ALIBABA_CLOUD_ACCESS_KEY_ID` | Access Key ID |
| `ALIBABA_CLOUD_ACCESS_KEY_SECRET` | Access Key Secret |
| `ALIBABA_CLOUD_SECURITY_TOKEN` | Security Token |
| `ALIBABA_CLOUD_ROLE_ARN` | RAM Role ARN |
| `ALIBABA_CLOUD_OIDC_PROVIDER_ARN` | OIDC Provider ARN |
| `ALIBABA_CLOUD_OIDC_TOKEN_FILE` | Path to OIDC token file |
| `ALIBABA_CLOUD_ROLE_SESSION_NAME` | Role session name |
| `ALIBABA_CLOUD_ECS_METADATA` | ECS RAM role name |
| `ALIBABA_CLOUD_ECS_METADATA_DISABLED` | Disable ECS metadata service |
| `ALIBABA_CLOUD_IMDSV1_DISABLE` | Disable IMDSv1 |
| `ALIBABA_CLOUD_CREDENTIALS_URI` | URL to fetch credentials |
| `ALIBABA_CLOUD_STS_REGION` | STS region |
| `ALIBABA_CLOUD_VPC_ENDPOINT_ENABLED` | Enable VPC endpoint |

## Issues

[Submit Issue](https://github.com/aliyun/credentials-cpp/issues/new/choose), Problems that do not meet the guidelines may be closed immediately.

## Release Notes

Detailed changes for each version are recorded in the [Release Notes](/CHANGELOG.md).

## Related

- [OpenAPI Developer Portal][open-api]
- [Latest Release][latest-release]
- [Alibaba Cloud Console][console]
- [Alibaba Cloud Home Page][aliyun]

## License

[Apache-2.0](/LICENSE.md)

Copyright (c) 2009-present, Alibaba Cloud All rights reserved.

[open-api]: https://next.api.aliyun.com
[latest-release]: https://github.com/aliyun/credentials-cpp/releases
[console]: https://home.console.aliyun.com
[aliyun]: https://www.aliyun.com
[ak]: https://usercenter.console.aliyun.com/#/manage/ak
[ram]: https://ram.console.aliyun.com/users
[permissions]: https://ram.console.aliyun.com/permissions
[RAM Role]: https://ram.console.aliyun.com/#/role/list
[policy]: https://www.alibabacloud.com/help/doc-detail/28664.htm
