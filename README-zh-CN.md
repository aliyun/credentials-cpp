[English](/README.md) | 简体中文

# Alibaba Cloud Credentials for C++
[![codecov](https://codecov.io/gh/aliyun/credentials-cpp/branch/master/graph/badge.svg)](https://codecov.io/gh/aliyun/credentials-cpp)
[![Travis Build Status](https://travis-ci.org/aliyun/credentials-cpp.svg?branch=master)](https://travis-ci.org/aliyun/credentials-cpp)

![](https://aliyunsdk-pages.alicdn.com/icons/AlibabaCloud.svg)

Alibaba Cloud Credentials for C++ 是帮助 C++ 开发者管理凭据的工具。

## 环境要求

### 编译器要求

- **Windows**: Visual Studio 2015 或以上版本
- **Linux**: GCC 4.9 或以上版本
- **macOS**: Clang (Xcode Command Line Tools)

### 构建工具

- **CMake**: 3.5 或以上版本（推荐 3.10+）
- **C++ 标准**: C++11 或更高版本

### 系统要求

- **内存**: 建议 4GB 或以上
- **磁盘空间**: 建议至少 500MB 可用空间

### 依赖库

- **OpenSSL**: 用于加密和网络通信
  - Windows: 通过 vcpkg 或 chocolatey 安装
  - Linux: `sudo apt-get install libssl-dev` (Ubuntu/Debian) 或 `sudo yum install openssl-devel` (CentOS/RHEL)
  - macOS: `brew install openssl`

- **nlohmann_json** (可选): 用于 CLIProfileProvider 的 JSON 支持
  - Windows: `vcpkg install nlohmann-json`
  - Linux: `sudo apt-get install nlohmann-json3-dev`
  - macOS: `brew install nlohmann-json`

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
  * 进入 cmake_build 目录，使用 Visual Studio 打开 AlibabaCloud_credentials.sln 解决方案
  * 选择构建 `Release` 输出
  * 并打开配置管理器勾选 `INSTALL`
  * 构建 -> 生成解决方案

## 快速使用

在您开始之前，您需要注册阿里云帐户并获取您的[凭证](https://usercenter.console.aliyun.com/#/manage/ak)。

### 默认凭证提供者链（推荐）

如果您不设置凭证类型，客户端会按照如下顺序查找凭证：

1. **环境变量**：`ALIBABA_CLOUD_ACCESS_KEY_ID` 和 `ALIBABA_CLOUD_ACCESS_KEY_SECRET`
2. **OIDC RAM 角色**：通过环境变量 `ALIBABA_CLOUD_ROLE_ARN`、`ALIBABA_CLOUD_OIDC_PROVIDER_ARN` 和 `ALIBABA_CLOUD_OIDC_TOKEN_FILE` 配置
3. **配置文件**：`~/.alibabacloud/credentials.ini` 或 `~/.alibabacloud/credentials.json`
4. **ECS 实例 RAM 角色**：通过 ECS 实例元数据服务（IMDS）获取
5. **Credentials URI**：通过环境变量 `ALIBABA_CLOUD_CREDENTIALS_URI` 指定的 URL 获取

**使用默认凭证提供者链（最佳实践）：**

```c++
#include <alibabacloud/credential/Credential.hpp>

using namespace AlibabaCloud::Credential;

// 不指定任何配置，使用默认凭证提供者链
Client client;

// 获取凭证
auto credential = client.getCredential();
printf("AccessKeyId: %s\n", credential.accessKeyId().c_str());
printf("AccessKeySecret: %s\n", credential.accessKeySecret().c_str());
printf("Type: %s\n", credential.type().c_str());
```

**通过环境变量设置：**

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

**通过配置文件设置：**

创建文件 `~/.alibabacloud/credentials.ini`：

```ini
[default]
type = access_key
access_key_id = <your-access-key-id>
access_key_secret = <your-access-key-secret>
```

或创建文件 `~/.alibabacloud/credentials.json`（需要安装 nlohmann_json）：

```json
{
  "mode": "AK",
  "accessKeyId": "<your-access-key-id>",
  "accessKeySecret": "<your-access-key-secret>"
}
```

### 指定凭证类型

如果您需要明确指定凭证类型，可以使用以下方式：

#### AccessKey

通过[用户信息管理][ak]设置 access_key，它们具有该账户完全的权限，请妥善保管。有时出于安全考虑，您不能把具有完全访问权限的主账户 AccessKey 交于一个项目的开发者使用，您可以[创建RAM子账户][ram]并为子账户[授权][permissions]，使用RAM子用户的 AccessKey 来进行API调用。

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

通过安全令牌服务（Security Token Service，简称 STS），申请临时安全凭证（Temporary Security Credentials，简称 TSC），创建临时安全凭证。

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

通过指定[RAM角色][RAM Role]，让凭证自动申请维护 STS Token。你可以通过为 `Policy` 赋值来限制获取到的 STS Token 的权限。

如果定义了环境变量 `ALIBABA_CLOUD_ECS_METADATA` 且不为空，程序会将该环境变量的值作为角色名称，请求 `http://100.100.100.200/latest/meta-data/ram/security-credentials/` 获取临时安全凭证作为默认凭证。

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

通过指定角色名称，让凭证自动申请维护 STS Token。

默认情况下，凭证工具会以安全强化模式（IMDSv2）访问 ECS 的元数据服务。如果抛出异常，凭证工具会切换到普通模式（IMDSv1）。您也可以配置 `disableIMDSv1` 参数或 `ALIBABA_CLOUD_IMDSV1_DISABLE` 环境变量来指定异常处理逻辑。取值如下：

- false（默认值）：凭证工具继续以普通模式（IMDSv1）获取访问凭证。
- true：抛出异常，凭证工具继续以安全强化模式获取访问凭证。

元数据服务器的配置决定了服务器是否支持安全强化模式（IMDSv2）。

此外，您可以指定 `ALIBABA_CLOUD_ECS_METADATA_DISABLED=true` 来禁止凭证工具访问 ECS 的元数据服务。

```c++
#include <alibabacloud.hpp>

using namespace AlibabaCloud_Credential;

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("ecs_ram_role")));
m.insert(pair<string, string*>("accessKeyId", new string("<AccessKeyId>")));
m.insert(pair<string, string*>("accessKeySecret", new string("<AccessKeySecret>")));
m.insert(pair<string, string*>("roleName", new string("<roleName>")));
// 可选。指定是否禁用IMDSv1。默认值：false
// m.insert(pair<string, string*>("disableIMDSv1", new string("true")));

auto *config = new Config(m);
Client client = Client(config);

printf("%s", client.getAccessKeyId().c_str());
printf("%s", client.getAccessKeySecret().c_str());
printf("%s", client.getRoleName().c_str());
```

#### RsaKeyPair

通过指定公钥Id和私钥文件，让凭证自动申请维护 AccessKey。仅支持日本站。

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

如呼叫中心(CCC)需用此凭证，请自行申请维护 Bearer Token。

```c++
#include <alibabacloud.hpp>

using namespace AlibabaCloud_Credential;

map<string, string*> m;
m.insert(pair<string, string*>("type", new string("bearer_token")));
m.insert(pair<string, string*>("bearerToken", new string("<BearerToken>")));

auto *config = new Config(m);
Client client = Client(config);

printf("%s", client.getBearerToken().c_str());
```

#### CloudSSO

通过阿里云SSO托管的角色凭证。

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

使用OAuth 2.0协议获取访问凭证。

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

## 问题

[提交 Issue](https://github.com/aliyun/credentials-cpp/issues/new/choose)，不符合指南的问题可能会立即关闭。

## 发行说明

每个版本的详细更改记录在[发行说明](/CHANGELOG.md)中。

## 相关

- [OpenAPI 开发者门户][open-api]
- [Latest Release][latest-release]
- [AlibabaCloud Console System][console]
- [Alibaba Cloud Home Page][aliyun]

## 许可证

[Apache-2.0](/LICENSE.md)

Copyright (c) 2009-present, Alibaba Cloud All rights reserved.

[open-api]: https://next.api.aliyun.com
[latest-release]: https://github.com/aliyun/credentials-cpp/releases
[console]: https://home.console.aliyun.com
[aliyun]: https://www.aliyun.com
