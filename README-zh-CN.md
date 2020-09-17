[English](/README.md) | 简体中文

# Alibaba Cloud Credentials for C++
[![codecov](https://codecov.io/gh/aliyun/credentials-cpp/branch/master/graph/badge.svg)](https://codecov.io/gh/aliyun/credentials-cpp)
[![Travis Build Status](https://travis-ci.org/aliyun/credentials-cpp.svg?branch=master)](https://travis-ci.org/aliyun/credentials-cpp)

![](https://aliyunsdk-pages.alicdn.com/icons/AlibabaCloud.svg)

Alibaba Cloud Credentials for C++ 是帮助 C++ 开发者管理凭据的工具。

## 安装

### Linux

```bash
git clone https://github.com/aliyun/credentials-cpp.git
cd credentials-cpp
sh scripts/install.sh
```

### Windows

1. 使用 git-bash 执行以下命令，从 Github 克隆源码

  ```bash
  git clone https://github.com/aliyun/credentials-cpp.git
  ```

2. 使用 Visual Studio 进行编译:
  * 在根目录下创建 `cmake_build` 目录
  * 打开 cmake-gui , 然后进行以下操作
    * 选择 `Browse Source` 为 `源代码目录(credentials-cpp)`
    * 选择 `Browse build` 为 `构建目录(cmake_build)`
    * 点击 `configure`
    * 点击 `generate`, 构建 VS 解决方案。

3. 编译安装 C++ SDK
  * 进入 cmake_build 目录，使用 Visual Studio 打开 alibabacloud_credentials.sln 解决方案
  * 选择构建 `Release` 输出
  * 并打开配置管理器勾选 `INSTALL`
  * 构建 -> 生成解决方案

## 快速使用

在您开始之前，您需要注册阿里云帐户并获取您的[凭证](https://usercenter.console.aliyun.com/#/manage/ak)。

### 凭证类型

#### AccessKey

通过[用户信息管理][ak]设置 access_key，它们具有该账户完全的权限，请妥善保管。有时出于安全考虑，您不能把具有完全访问权限的主账户 AccessKey 交于一个项目的开发者使用，您可以[创建RAM子账户][ram]并为子账户[授权][permissions]，使用RAM子用户的 AccessKey 来进行API调用。

```c++
#include <alibabacloud/credential.hpp>

using namespace Alibabacloud_Credential;

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

通过安全令牌服务（Security Token Service，简称 STS），申请临时安全凭证（Temporary Security Credentials，简称 TSC），创建临时安全凭证。

```c++
#include <alibabacloud/credential.hpp>

using namespace Alibabacloud_Credential;

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

通过指定[RAM角色][RAM Role]，让凭证自动申请维护 STS Token。你可以通过为 `Policy` 赋值来限制获取到的 STS Token 的权限。

如果定义了环境变量 `ALIBABA_CLOUD_ECS_METADATA` 且不为空，程序会将该环境变量的值作为角色名称，请求 `http://100.100.100.200/latest/meta-data/ram/security-credentials/` 获取临时安全凭证作为默认凭证。

```c++
#include <alibabacloud/credential.hpp>

using namespace Alibabacloud_Credential;

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

通过指定角色名称，让凭证自动申请维护 STS Token

```c++
#include <alibabacloud/credential.hpp>

using namespace Alibabacloud_Credential;

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("ecs_ram_role")));
m.insert(pair<string, string*>("accessKeyId", new string("<AccessKeyId>")));
m.insert(pair<string, string*>("accessKeySecret", new string("<AccessKeySecret>")));
m.insert(pair<string, string*>("roleName", new string("<roleName>")));

auto *config = new Config(m);
Client client = Client(config);

printf("%s", client.getAccessKeyId().c_str());
printf("%s", client.getAccessKeySecret().c_str());
printf("%s", client.getRoleName().c_str());
```

#### RsaKeyPair

通过指定公钥Id和私钥文件，让凭证自动申请维护 AccessKey。仅支持日本站。

```c++
#include <alibabacloud/credential.hpp>

using namespace Alibabacloud_Credential;

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

如呼叫中心(CCC)需用此凭证，请自行申请维护 Bearer Token。

```c++
#include <alibabacloud/credential.hpp>

using namespace Alibabacloud_Credential;

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("bearer_token")));
m.insert(pair<string, string*>("bearerToken", new string("<BearerToken>")));

auto *config = new Config(m);
Client client = Client(config);

printf("%s", client.getBearerToken().c_str());
```

## 问题

[提交 Issue](https://github.com/aliyun/credentials-cpp/issues/new/choose)，不符合指南的问题可能会立即关闭。

## 发行说明

每个版本的详细更改记录在[发行说明](/CHANGELOG.md)中。

## 相关

- [OpenAPI Explorer][open-api]
- [Latest Release][latest-release]
- [Alibabacloud Console System][console]
- [Alibaba Cloud Home Page][aliyun]

## 许可证

[Apache-2.0](/LICENSE.md)

Copyright (c) 2009-present, Alibaba Cloud All rights reserved.

[open-api]: https://api.aliyun.com
[latest-release]: https://github.com/aliyun/credentials-cpp/releases
[console]: https://home.console.aliyun.com
[aliyun]: https://www.aliyun.com
