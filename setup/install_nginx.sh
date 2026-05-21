#!/bin/bash

# Set versions and paths
ALIAS=$1

NGINX_VERSION=1.28.0

INSTALL_DIR=$HOME/nginx

PORT=8080

echo "🔧 Creating install directory at $INSTALL_DIR"

mkdir -p $INSTALL_DIR

# Download Nginx source

echo "⬇️ Downloading Nginx $NGINX_VERSION..."

wget http://nginx.org/download/nginx-$NGINX_VERSION.tar.gz

tar -xzf nginx-$NGINX_VERSION.tar.gz

cd nginx-$NGINX_VERSION || exit 1

# Configure build to install locally

echo "⚙️ Configuring Nginx..."

./configure --prefix="$INSTALL_DIR" --without-http_gzip_module --with-http_ssl_module --with-openssl=$(which openssl)

# Compile and install

echo "🛠 Building Nginx..."

make

make install

# Modify nginx.conf to use non-privileged port

CONF="$INSTALL_DIR/conf/nginx.conf"

echo "✏️ Updating Nginx config to use port $PORT..."

sed -i "s/listen       80;/listen       $PORT;/g" "$CONF"

echo "✅ Nginx installed successfully in $INSTALL_DIR"

mkdir -p $INSTALL_DIR/sites-available

# echo "➡️ To start Nginx: $INSTALL_DIR/sbin/nginx"
echo "➡️ To start Nginx: tiramisu webserver start --alias=$ALIAS"

# echo "➡️ To stop Nginx: $INSTALL_DIR/sbin/nginx -s stop"
echo "➡️ To stop Nginx: tiramisu webserver stop --alias=$ALIAS"

echo "➡️ To restart Nginx: tiramisu webserver restart --alias=$ALIAS"

# echo "🌐 Nginx will listen on http://localhost:$PORT"