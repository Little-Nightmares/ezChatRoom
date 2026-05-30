#!/bin/bash
# ChatRoom 服务端 - Ubuntu 构建脚本
# 用法: chmod +x build_ubuntu_server.sh && ./build_ubuntu_server.sh

set -e

echo "============================================"
echo " ChatRoom 服务端 — Ubuntu 构建"
echo "============================================"

# 1. 安装依赖
echo "[1/5] 安装编译依赖..."
sudo apt update
sudo apt install -y build-essential cmake pkg-config \
    qt6-base-dev libqt6sql6-sqlite libssl-dev

# 2. 创建构建目录
echo "[2/5] 创建构建目录..."
mkdir -p build-server && cd build-server

# 3. CMake 配置
echo "[3/5] CMake 配置..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# 4. 只编译服务端
echo "[4/5] 编译服务端..."
make -j$(nproc) ChatRoomServer

echo "  ✓ 服务端: $(realpath server/ChatRoomServer)"

# 5. 创建部署目录
echo "[5/5] 创建部署目录..."
cd server
mkdir -p uploads/images uploads/files uploads/avatars

# 写入 systemd 服务文件
cat > chatroom-server.service << 'SERVICEEOF'
[Unit]
Description=ChatRoom Server
After=network.target

[Service]
Type=simple
WorkingDirectory=/opt/chatroom
ExecStart=/opt/chatroom/ChatRoomServer --port 6667 --admin-port 19527
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
SERVICEEOF

cat > start.sh << 'EOF'
#!/bin/bash
DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR"
echo "[INFO] Starting ChatRoom Server..."
echo "[INFO] Admin: http://localhost:19527"
./ChatRoomServer
EOF
chmod +x start.sh

echo ""
echo "============================================"
echo " 构建完成！"
echo ""
echo " 二进制: $(pwd)/ChatRoomServer"
echo " 快速启动: ./start.sh"
echo " systemd: sudo cp chatroom-server.service /etc/systemd/system/"
echo "============================================"
