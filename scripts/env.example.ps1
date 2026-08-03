# env.example.ps1
# 本机环境配置模板。
# 用法：复制为 env.local.ps1 并填入本机实际值；env.local.ps1 已被 .gitignore 忽略，不会提交。
# 仓库只记录版本，不记录机器路径。

# CCS Theia 安装根目录（含 theia/ccstudio.exe 的目录之上）
$env:CCS_HOME = "C:\ti\ccs"

# MSPM0 SDK 根目录
$env:MSPM0_SDK_ROOT = "C:\ti\mspm0_sdk_2_10_00_04"

# DSLite 烧录工具完整路径
$env:DSLITE_PATH = "C:\ti\ccs\ccs_base\DebugServer\bin\DSLite.exe"

# 调试/串口端口（按需填写，如 COM6）
$env:SERIAL_PORT = ""

# 本机 host 测试编译器（可选，默认 clang）
# $env:CC = "clang"
