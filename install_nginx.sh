#!/bin/bash

# Set versions and paths

NGINX_VERSION=1.25.4

INSTALL_DIR="$HOME/nginx"

PORT=8080

echo "🔧 Creating install directory at $INSTALL_DIR"

mkdir -p "$INSTALL_DIR"

# Download Nginx source

echo "⬇️ Downloading Nginx $NGINX_VERSION..."

wget http://nginx.org/download/nginx-$NGINX_VERSION.tar.gz

tar -xzf nginx-$NGINX_VERSION.tar.gz

cd nginx-$NGINX_VERSION || exit 1

# Configure build to install locally

echo "⚙️ Configuring Nginx..."

./configure --prefix="$INSTALL_DIR" --with-http_ssl_module

# Compile and install

echo "🛠 Building Nginx..."

make

make install

# Modify nginx.conf to use non-privileged port

CONF="$INSTALL_DIR/conf/nginx.conf"

echo "✏️ Updating Nginx config to use port $PORT..."

sed -i "s/listen 80;/listen $PORT;/" "$CONF"

echo "✅ Nginx installed successfully in $INSTALL_DIR"

echo "➡️ To start Nginx: $INSTALL_DIR/sbin/nginx"

echo "➡️ To stop Nginx: $INSTALL_DIR/sbin/nginx -s stop"

echo "🌐 Nginx will listen on http://localhost:$PORT"