English | [简体中文](/README-zh-CN.md)

# Alibaba Cloud Credentials for C++
[![codecov](https://codecov.io/gh/aliyun/credentials-cpp/branch/master/graph/badge.svg)](https://codecov.io/gh/aliyun/credentials-cpp)
[![Travis Build Status](https://travis-ci.org/aliyun/credentials-cpp.svg?branch=master)](https://travis-ci.org/aliyun/credentials-cpp)

![](https://aliyunsdk-pages.alicdn.com/icons/AlibabaCloud.svg)

Alibaba Cloud Credentials for C++ is a tool that helps C++ developers manage their credentials.

## Requirements

### Compiler Requirements

- **Windows**: Visual Studio 2015 or later
- **Linux**: GCC 4.9 or later
- **macOS**: Clang (Xcode Command Line Tools)

### Build Tools

- **CMake**: 3.5 or later (3.10+ recommended)
- **C++ Standard**: C++11 or higher

### System Requirements

- **Memory**: 4GB or more recommended
- **Disk Space**: At least 500MB available space

### Dependencies

- **OpenSSL**: For encryption and network communication
  - Windows: Install via vcpkg or chocolatey
  - Linux: `sudo apt-get install libssl-dev` (Ubuntu/Debian) or `sudo yum install openssl-devel` (CentOS/RHEL)
  - macOS: `brew install openssl`

- **nlohmann_json** (Optional): For CLIProfileProvider JSON support
  - Windows: `vcpkg install nlohmann-json`
  - Linux: `sudo apt-get install nlohmann-json3-dev`
  - macOS: `brew install nlohmann-json`

## Installation

### Linux

```bash
git clone https://github.com/aliyun/credentials-cpp.git
cd credentials-cpp
sh scripts/install.sh
```

### Windows

1. Run the following command to clone code from Github via git-bash:

  ```bash
  git clone https://github.com/aliyun/credentials-cpp.git
  ```

2. Build Visual Studio solution
  * Change directory to source code and make directory `cmake_build`
  * Open CMake UI and
    * `Browse Source` to open source code directory.
    * `Browse build`  to open the created `cmake_build` directory
    * click `configure`
    * click `generate`, Generate VS solution

3. Build and Install C++ SDK
  * Go to the cmake_build directory and open AlibabaCloud_credentials.sln with Visual Studio Solutions
  * Select  `Release`
  * Check INSTALL option from Build -> Configuration Manager
  * Build->Build Solutions to build.

### Command Line Build (Cross-platform)

To build without the CMake GUI, run the commands below from the repo root (`credentials-cpp` directory that contains `CMakeLists.txt`). Always pair `-S` with the source directory and `-B` with the build directory so CMake never falls back to `/`.

- **macOS/Linux (bash/zsh):**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_UNIT_TESTS=ON
cmake --build build --config Release
```

- **Windows PowerShell:** PowerShell does not treat `\` as a line continuation, so keep the command on one line or use the backtick (`` ` ``) continuation.

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_UNIT_TESTS=ON
cmake --build build --config Release
```

If you prefer multi-line PowerShell commands, write them as:

```powershell
cmake `
  -S . `
  -B build `
  -DCMAKE_BUILD_TYPE=Release `
  -DENABLE_UNIT_TESTS=ON
```

The first `cmake` command configures the project, while `cmake --build` compiles (passing `--config` is required on multi-config generators like Visual Studio).

## Quick Examples

Before you begin, you need to sign up for an Alibaba Cloud account and retrieve your [Credentials](https://usercenter.console.aliyun.com/#/manage/ak).

### Default Credentials Provider Chain (Recommended)

If you do not specify a credential type, the client will search for credentials in the following order:

1. **Environment Variables**: `ALIBABA_CLOUD_ACCESS_KEY_ID` and `ALIBABA_CLOUD_ACCESS_KEY_SECRET`
2. **OIDC RAM Role**: Configured via environment variables `ALIBABA_CLOUD_ROLE_ARN`, `ALIBABA_CLOUD_OIDC_PROVIDER_ARN`, and `ALIBABA_CLOUD_OIDC_TOKEN_FILE`
3. **Configuration File**: `~/.alibabacloud/credentials.ini` or `~/.alibabacloud/credentials.json`
4. **ECS Instance RAM Role**: Retrieved via ECS Instance Metadata Service (IMDS)
5. **Credentials URI**: Retrieved from URL specified by environment variable `ALIBABA_CLOUD_CREDENTIALS_URI`

**Using Default Credentials Provider Chain (Best Practice):**

```c++
#include <alibabacloud/credential/Credential.hpp>

using namespace AlibabaCloud::Credential;

// Use default credentials provider chain without specifying any configuration
Client client;

// Get credentials
auto credential = client.getCredential();
printf("AccessKeyId: %s\n", credential.accessKeyId().c_str());
printf("AccessKeySecret: %s\n", credential.accessKeySecret().c_str());
printf("Type: %s\n", credential.type().c_str());
```

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

**Setting credentials via configuration file:**

Create file `~/.alibabacloud/credentials.ini`:

```ini
[default]
type = access_key
access_key_id = <your-access-key-id>
access_key_secret = <your-access-key-secret>
```

Or create file `~/.alibabacloud/credentials.json` (requires nlohmann_json):

```json
{
  "mode": "AK",
  "accessKeyId": "<your-access-key-id>",
  "accessKeySecret": "<your-access-key-secret>"
}
```

### Specifying Credential Type

If you need to explicitly specify a credential type, you can use the following methods:

#### AccessKey

Setup access_key credential through [User Information Management][ak], it have full authority over the account, please keep it safe. Sometimes for security reasons, you cannot hand over a primary account AccessKey with full access to the developer of a project. You may create a sub-account [RAM Sub-account][ram] , grant its [authorization][permissions]，and use the AccessKey of RAM Sub-account.

```c++
#include <alibabacloud.hpp>

using namespace AlibabaCloud_Credential;

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("access_key")));
m.insert(pair<string, string*>("accessKeyId", new string("<AccessKeyId>")));
m.insert(pair<string, string*>("accessKeySecret", new string("<AccessKeySecret>")));

auto *config = new Config(m);
Client client = Client(config);

printf("%s", client.getAccessKeyId().c_str());
printf("%s", client.getAccessKeySecret().c_str());
```

#### STS

Create a temporary security credential by applying Temporary Security Credentials (TSC) through the Security Token Service (STS).

```c++
#include <alibabacloud.hpp>

using namespace AlibabaCloud_Credential;

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("sts")));
m.insert(pair<string, string*>("accessKeyId", new string("<AccessKeyId>")));
m.insert(pair<string, string*>("accessKeySecret", new string("<AccessKeySecret>")));
m.insert(pair<string, string*>("securityToken", new string("<SecurityToken>")));

auto *config = new Config(m);
Client client = Client(config);

printf("%s", client.getAccessKeyId().c_str());
printf("%s", client.getAccessKeySecret().c_str());
printf("%s", client.getSecurityToken().c_str());
```

#### RamRoleArn

By specifying [RAM Role][RAM Role], the credential will be able to automatically request maintenance of STS Token. If you want to limit the permissions([How to make a policy][policy]) of STS Token, you can assign value for `Policy`.

If the environment variable `ALIBABA_CLOUD_ECS_METADATA` is defined and not empty, the program will take the value of the environment variable as the role name and request `http://100.100.100.200/latest/meta-data/ram/security-credentials/` to get the temporary Security credentials are used as default credentials.

```c++
#include <alibabacloud.hpp>

using namespace AlibabaCloud_Credential;

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("ram_role_arn")));
m.insert(pair<string, string*>("accessKeyId", new string("<AccessKeyId>")));
m.insert(pair<string, string*>("accessKeySecret", new string("<AccessKeySecret>")));
m.insert(pair<string, string*>("roleArn", new string("<RoleArn>")));
m.insert(pair<string, string*>("roleSessionName", new string("<RoleSessionName>")));
m.insert(pair<string, string*>("policy", new string("<Policy>")));

auto *config = new Config(m);
Client client = Client(config);

printf("%s", client.getAccessKeyId().c_str());
printf("%s", client.getAccessKeySecret().c_str());
printf("%s", client.getRoleArn().c_str());
printf("%s", client.getRoleSessionName().c_str());
printf("%s", client.getPolicy().c_str());
```

#### EcsRamRole

By specifying the role name, the credential will be able to automatically request maintenance of STS Token.

By default, the Credentials tool accesses the metadata server of ECS in security hardening mode (IMDSv2). If an exception is thrown, the Credentials tool switches to the normal mode (IMDSv1). You can also configure the `disableIMDSv1` parameter or the `ALIBABA_CLOUD_IMDSV1_DISABLE` environment variable to specify the exception handling logic. Valid values:

- false (default): The Credentials tool continues to obtain the access credential in normal mode (IMDSv1).
- true: The exception is thrown and the Credentials tool continues to obtain the access credential in security hardening mode.

The configurations for the metadata server determine whether the server supports the security hardening mode (IMDSv2).

In addition, you can specify `ALIBABA_CLOUD_ECS_METADATA_DISABLED=true` to disable access from the Credentials tool to the metadata server of ECS.

```c++
#include <alibabacloud.hpp>

using namespace AlibabaCloud_Credential;

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("ecs_ram_role")));
m.insert(pair<string, string*>("accessKeyId", new string("<AccessKeyId>")));
m.insert(pair<string, string*>("accessKeySecret", new string("<AccessKeySecret>")));
m.insert(pair<string, string*>("roleName", new string("<RoleName>")));
// Optional. Specifies whether to disable IMDSv1. Default value: false
// m.insert(pair<string, string*>("disableIMDSv1", new string("true")));

auto *config = new Config(m);
Client client = Client(config);

printf("%s", client.getAccessKeyId().c_str());
printf("%s", client.getAccessKeySecret().c_str());
printf("%s", client.getRoleName().c_str());
```

#### RsaKeyPair

By specifying the public key Id and the private key file, the credential will be able to automatically request maintenance of the AccessKey before sending the request. Only Japan station is supported.

```c++
#include <alibabacloud.hpp>

using namespace AlibabaCloud_Credential;

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("rsa_key_pair")));
m.insert(pair<string, string*>("publicKeyId", new string("<PublicKeyId>")));
m.insert(pair<string, string*>("privateKeyFile", new string("<PrivateKeyFile>")));

auto *config = new Config(m);
Client client = Client(config);

printf("%s", client.getPublicKeyId().c_str());
printf("%s", client.getPrivateKey().c_str());
```

#### Bearer Token

If credential is required by the Cloud Call Centre (CCC), please apply for Bearer Token maintenance by yourself.

```c++
#include <alibabacloud.hpp>

using namespace AlibabaCloud_Credential;

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("bearer_token")));
m.insert(pair<string, string*>("bearerToken", new string(new string("<BearerToken>"))));

auto *config = new Config(m);
Client client = Client(config);

printf("%s", client.getBearerToken().c_str());
```

#### CloudSSO

Use Alibaba Cloud SSO managed role credentials.

```c++
#include <alibabacloud.hpp>

using namespace AlibabaCloud_Credential;

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("sso")));
m.insert(pair<string, string*>("roleName", new string("<RoleName>")));
m.insert(pair<string, string*>("regionId", new string("cn-hangzhou")));

auto *config = new Config(m);
Client client = Client(config);

printf("%s", client.getAccessKeyId().c_str());
printf("%s", client.getAccessKeySecret().c_str());
printf("%s", client.getSecurityToken().c_str());
```

#### OAuth

Use OAuth 2.0 protocol to obtain access credentials.

```c++
#include <alibabacloud.hpp>

using namespace AlibabaCloud_Credential;

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("oauth")));
m.insert(pair<string, string*>("accessKeyId", new string("<ClientId>")));
m.insert(pair<string, string*>("accessKeySecret", new string("<ClientSecret>")));
m.insert(pair<string, string*>("stsEndpoint", new string("https://oauth.aliyuncs.com/v1/token")));
m.insert(pair<string, string*>("regionId", new string("cn-hangzhou")));

auto *config = new Config(m);
Client client = Client(config);

printf("%s", client.getBearerToken().c_str());
```

## Issue

[Submit Issue](https://github.com/aliyun/credentials-cpp/issues/new/choose), Problems that do not meet the guidelines may close immediately.

## Release notes

Detailed changes for each version are recorded in the [Release Notes](/CHANGELOG.md).

## Related

* [OpenAPI Developer Portal][open-api]
* [Latest Release][latest-release]
* [AlibabaCloud Console System][console]
* [Alibaba Cloud Home Page][aliyun]

## License

[Apache-2.0](/LICENSE.md)

Copyright (c) 2009-present, Alibaba Cloud All rights reserved.

[open-api]: https://next.api.aliyun.com
[latest-release]: https://github.com/aliyun/credentials-cpp/releases
[console]: https://home.console.aliyun.com
[aliyun]: https://www.aliyun.com
