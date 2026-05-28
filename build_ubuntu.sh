#!/bin/bash
# ChatRoom — Ubuntu 构建脚本
# 用法: chmod +x build_ubuntu.sh && ./build_ubuntu.sh

set -e

echo "============================================"
echo " ChatRoom — Ubuntu 构建"
echo "============================================"

# 1. 安装依赖
echo "[1/5] 安装编译依赖..."
sudo apt update
sudo apt install -y build-essential cmake pkg-config \
    qt6-base-dev qt6-webengine-dev qt6-multimedia-dev \
    libqt6sql6-sqlite libssl-dev libgl1-mesa-dev

# 2. 创建构建目录
echo "[2/5] 创建构建目录..."
mkdir -p build/ubuntu-release
cd build/ubuntu-release

# 3. CMake 配置
echo "[3/5] CMake 配置..."
cmake -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="/usr/lib/x86_64-linux-gnu/cmake/Qt6" \
    ../..

# 4. 编译
echo "[4/5] 编译..."
NPROC=$(nproc)
make -j$NPROC ChatRoomServer ChatRoomClient
echo "  ✓ 服务端: $(realpath ChatRoomServer)"
echo "  ✓ 客户端: $(realpath ChatRoomClient)"

# 5. 打包
echo "[5/5] 打包..."
mkdir -p ChatRoom-Package/server ChatRoom-Package/client

# 复制二进制
cp ChatRoomServer ChatRoom-Package/server/
cp ChatRoomClient ChatRoom-Package/client/

# 复制 OpenSSL 库
ldd ChatRoomServer 2>/dev/null | grep -i ssl | awk '{print $3}' | xargs -I{} cp {} ChatRoom-Package/server/ 2>/dev/null || true
ldd ChatRoomClient 2>/dev/null | grep -i ssl | awk '{print $3}' | xargs -I{} cp {} ChatRoom-Package/client/ 2>/dev/null || true

# 复制 admin.token（若存在）
cp ../../admin.token ChatRoom-Package/server/ 2>/dev/null || echo "  ⚠ admin.token 未找到"

# 写入启动脚本
cat > ChatRoom-Package/start_server.sh << 'EOF'
#!/bin/bash
DIR="$(cd "$(dirname "$0")/server" && pwd)"
cd "$DIR"
echo "[INFO] Starting ChatRoom Server..."
echo "[INFO] Admin: http://localhost:19527"
./ChatRoomServer
EOF

cat > ChatRoom-Package/start_client.sh << 'EOF'
#!/bin/bash
DIR="$(cd "$(dirname "$0")/client" && pwd)"
cd "$DIR"
echo "[INFO] Starting ChatRoom Client..."
./ChatRoomClient
EOF

chmod +x ChatRoom-Package/start_server.sh ChatRoom-Package/start_client.sh

# 打包 tar.gz
cd ChatRoom-Package
tar czf ../ChatRoom-Ubuntu-x86_64.tar.gz .
cd ..

echo ""
echo "============================================"
echo " 构建完成！"
echo " 发布包: $(realpath ChatRoom-Ubuntu-x86_64.tar.gz)"
echo ""
echo " 使用方式:"
echo "   tar xzf ChatRoom-Ubuntu-x86_64.tar.gz"
echo "   cd ChatRoom-Package"
echo "   ./start_server.sh   # 启动服务端"
echo "   ./start_client.sh    # 启动客户端"
echo "============================================"
