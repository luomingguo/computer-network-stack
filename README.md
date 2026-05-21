# 简单网络协议栈实现





## 环境搭建



1. 安装虚拟机 [VirtualBox](https://www.virtualbox.org/)
   - 如果主机是Linux，这个跳过，直接运行即可，缺啥装啥
   - Debian类型的（比如Ubuntu），执行`apt-get install virtualbox`进行安装
   - 如果是Macbook Apple芯片，可以考虑改用 [UTM 虚拟机](https://mac.getutm.app/)
2. 下载 [镜像](https://stanford.edu/class/cs144/vm_files/cs144-fall-2025-x86.ova)， 配置时建议内存给够4GB，CPU核至少一个（多的话利于编译）
   - ssh端口为2222， 账密分别都是cs1444
   - 镜像里面已经下载并配置好工具链了
3. 本人vscode编译环境，习惯用sftp同步代码运行。





## 编译与运行

构建： `cmake -S . -B build`
- 如果是macbook arm64玩家运行会非常慢，更换成Clang++编译器， 比如`cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++`

编译： `cmake --build build`

运行测试：`cmake --build build --target test`

性能基准测试：`cmake --build build --target speed`

获取 C++ 编译器改进建议： `cmake --build build --target tidy`

规范代码：`cmake --build build --target format`

关于调试：

- `gdb tests/byte_stream_one_write`
- 如果



## 功能特性



## 性能基准
