English | [简体中文](/README-zh-CN.md)

# Alibaba Cloud Credentials for C++
[![codecov](https://codecov.io/gh/aliyun/credentials-cpp/branch/master/graph/badge.svg)](https://codecov.io/gh/aliyun/credentials-cpp)
[![Travis Build Status](https://travis-ci.org/aliyun/credentials-cpp.svg?branch=master)](https://travis-ci.org/aliyun/credentials-cpp)

![](https://aliyunsdk-pages.alicdn.com/icons/AlibabaCloud.svg)

Alibaba Cloud Credentials for C++ is a tool that helps C++ developers manage their credentials.

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
  * Go to the cmake_build directory and open alibabacloud_credentials.sln with Visual Studio Solutions
  * Select  `Release`
  * Check INSTALL option from Build -> Configuration Manager
  * Build->Build Solutions to build.

## Quick Examples

Before you begin, you need to sign up for an Alibaba Cloud account and retrieve your [Credentials](https://usercenter.console.aliyun.com/#/manage/ak).

### Credential Type

#### AccessKey

Setup access_key credential through [User Information Management][ak], it have full authority over the account, please keep it safe. Sometimes for security reasons, you cannot hand over a primary account AccessKey with full access to the developer of a project. You may create a sub-account [RAM Sub-account][ram] , grant its [authorization][permissions]，and use the AccessKey of RAM Sub-account.

```c++
#include <alibabacloud/credential.hpp>

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("access_key")));
m.insert(pair<string, string*>("accessKeyId", new string("<AccessKeyId>")));
m.insert(pair<string, string*>("accessKeySecret", new string("<AccessKeySecret>")));

auto *config = new Alibabacloud_Credential::Config(m);
auto *client = new Alibabacloud_Credential::Client(config);

printf("%s", *client.getAccessKeyId().c_str());
printf("%s", *client.getAccessKeySecret().c_str());
```

#### STS

Create a temporary security credential by applying Temporary Security Credentials (TSC) through the Security Token Service (STS).

```c++
#include <alibabacloud/credential.hpp>

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("sts")));
m.insert(pair<string, string*>("accessKeyId", new string("<AccessKeyId>")));
m.insert(pair<string, string*>("accessKeySecret", new string("<AccessKeySecret>")));
m.insert(pair<string, string*>("securityToken", new string("<SecurityToken>")));

auto *config = new Alibabacloud_Credential::Config(m);
auto *client = new Alibabacloud_Credential::Client(config);

printf("%s", *client.getAccessKeyId().c_str());
printf("%s", *client.getAccessKeySecret().c_str());
printf("%s", *client.getSecurityToken().c_str());
```

#### RamRoleArn

By specifying [RAM Role][RAM Role], the credential will be able to automatically request maintenance of STS Token. If you want to limit the permissions([How to make a policy][policy]) of STS Token, you can assign value for `Policy`.

If the environment variable `ALIBABA_CLOUD_ECS_METADATA` is defined and not empty, the program will take the value of the environment variable as the role name and request `http://100.100.100.200/latest/meta-data/ram/security-credentials/` to get the temporary Security credentials are used as default credentials.

```c++
#include <alibabacloud/credential.hpp>

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("ram_role_arn")));
m.insert(pair<string, string*>("accessKeyId", new string("<AccessKeyId>")));
m.insert(pair<string, string*>("accessKeySecret", new string("<AccessKeySecret>")));
m.insert(pair<string, string*>("roleArn", new string("<RoleArn>")));
m.insert(pair<string, string*>("roleSessionName", new string("<RoleSessionName>")));
m.insert(pair<string, string*>("policy", new string("<Policy>")));

auto *config = new Alibabacloud_Credential::Config(m);
auto *client = new Alibabacloud_Credential::Client(config);

printf("%s", *client.getAccessKeyId().c_str());
printf("%s", *client.getAccessKeySecret().c_str());
printf("%s", *client.getRoleArn().c_str());
printf("%s", *client.getRoleSessionName().c_str());
printf("%s", *client.getPolicy().c_str());
```

#### EcsRamRole

By specifying the role name, the credential will be able to automatically request maintenance of STS Token.

```c++
#include <alibabacloud/credential.hpp>

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("ecs_ram_role")));
m.insert(pair<string, string*>("accessKeyId", new string("<AccessKeyId>")));
m.insert(pair<string, string*>("accessKeySecret", new string("<AccessKeySecret>")));
m.insert(pair<string, string*>("roleName", new string("<RoleName>")));

auto *config = new Alibabacloud_Credential::Config(m);
auto *client = new Alibabacloud_Credential::Client(config);

printf("%s", *client.getAccessKeyId().c_str());
printf("%s", *client.getAccessKeySecret().c_str());
printf("%s", *client.getRoleName().c_str());
```

#### RsaKeyPair

By specifying the public key Id and the private key file, the credential will be able to automatically request maintenance of the AccessKey before sending the request. Only Japan station is supported.

```c++
#include <alibabacloud/credential.hpp>

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("rsa_key_pair")));
m.insert(pair<string, string*>("publicKeyId", new string("<PublicKeyId>")));
m.insert(pair<string, string*>("privateKeyFile", new string("<PrivateKeyFile>")));

auto *config = new Alibabacloud_Credential::Config(m);
auto *client = new Alibabacloud_Credential::Client(config);

printf("%s", *client.getPublicKeyId().c_str());
printf("%s", *client.getPrivateKey().c_str());
```

#### Bearer Token

If credential is required by the Cloud Call Centre (CCC), please apply for Bearer Token maintenance by yourself.

```c++
#include <alibabacloud/credential.hpp>

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("bearer_token")));
m.insert(pair<string, string*>("bearerToken", new string(new string("<BearerToken>"))));

auto *config = new Alibabacloud_Credential::Config(m);
auto *client = new Alibabacloud_Credential::Client(config);

printf("%s", *client.getBearerToken().c_str());
```

## Issue

[Submit Issue](https://github.com/aliyun/credentials-cpp/issues/new/choose), Problems that do not meet the guidelines may close immediately.

## Release notes

Detailed changes for each version are recorded in the [Release Notes](/CHANGELOG.md).

## Related

* [OpenAPI Explorer][open-api]
* [Latest Release][latest-release]
* [Alibabacloud Console System][console]
* [Alibaba Cloud Home Page][aliyun]

## License

[Apache-2.0](/LICENSE.md)

Copyright (c) 2009-present, Alibaba Cloud All rights reserved.

[open-api]: https://api.aliyun.com
[latest-release]: https://github.com/aliyun/credentials-cpp/releases
[console]: https://home.console.aliyun.com
[aliyun]: https://www.aliyun.com
