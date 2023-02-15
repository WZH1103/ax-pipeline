# 源码编译（Linux）

ax-pipeline 的源码编译目前有两种实现路径：

- 基于 AX-Pi 的本地编译，因为 AX-Pi 上集成的完成了软件开发环境，操作简单；
- 嵌入式 Linux 交叉编译。

## 本地编译

### 硬件需求

- AX-Pi（基于 AX620A，面向社区开发者的高性价比开发板）

### 编译环境
- cmake 版本大于等于 3.13
- AX620A/U 配套的交叉编译工具链 `arm-linux-gnueabihf-gxx` 已添加到环境变量中

### 编译过程
- 新手用户请确保以下每一步的所有命令都成功运行无误再进行下一步命令
- 如果单独某一部分的命令出错，请及时在技术交流 QQ 群反馈，QQ 群:139953715
- 反馈时请清楚的描述遇到的问题以及错误

1、git clone 下载源码和子模块,进入文件夹
```shell
git clone https://github.com/WZH1103/ax-pipeline.git --recursive

cd ax-pipeline/
```
2、创建 3rdparty，下载opencv
```shell
mkdir 3rdparty
cd 3rdparty
wget https://github.com/AXERA-TECH/ax-samples/releases/download/v0.1/opencv-arm-linux-gnueabihf-gcc-7.5.0.zip
unzip opencv-arm-linux-gnueabihf-gcc-7.5.0.zip
```
3、下载并配置交叉编译工具链（如果已经配置并确定可用，这一部分可以跳过）
```shell
# 下载工具链
wget http://releases.linaro.org/components/toolchain/binaries/7.5-2019.12/arm-linux-gnueabihf/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf.tar.xz

# 解压工具链到opt文件夹中
sudo tar -Jxvf gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf.tar.xz -C /opt/

# 修改环境变量
sudo gedit /etc/profile

# 将下面内容保存到文件最后一行并重启系统
export PATH=$PATH:/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin
```
4、创建 build 目录，并创建 cmake 编译任务
```shell
cd ..
mkdir build
cd build
cmake -DSIPY_BUILD=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../toolchains/arm-linux-gnueabihf.toolchain.cmake -DCMAKE_INSTALL_PREFIX=install ..
make -j8
make install
```

5、编译完成后，生成的可执行示例存放在 `ax-pipeline/build/install/bin/` 路径下：

```shell
ax-pipeline/build$ tree install
install
└── bin
    ├── config
    │   ├── ax_person_det.json
    │   ├── ax_pose.json
    │   ├── ax_pose_yolov5s.json
    │   ├── hand_pose.json
    │   ├── hrnet_pose.json
    │   ├── nanodet.json
    │   ├── palm_hand_detection.json
    │   ├── pp_human_seg.json
    │   ├── yolov5_seg.json
    │   ├── yolov5s.json
    │   ├── yolov5s_face.json
    │   ├── yolov7.json
    │   └── yolox.json
    └── my_proj
```
