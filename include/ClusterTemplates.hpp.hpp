#pragma once
#include <string_view>

namespace Tiramisu::Templates {

// The embedded docker-compose.yml
constexpr std::string_view DOCKER_COMPOSE = R"yaml(
version: '3.8'

services:
  env-alpha:
    build:
      context: .
      dockerfile: Dockerfile.runtime
    container_name: tiramisu-env-alpha
    ports:
      - "8081:80"
      - "2221:22"
    volumes:
      - alpha_apps:/var/tiramisu/apps
    environment:
      - ENV_NAME=alpha
      - DB_HOST=db-alpha
      - DB_USER=tiramisu_user
      - DB_PASSWORD=tiramisu_pass
      - DB_NAME=tiramisu_alpha
    depends_on:
      - db-alpha

  db-alpha:
    image: postgres:16-alpine
    container_name: tiramisu-db-alpha
    environment:
      POSTGRES_USER: tiramisu_user
      POSTGRES_PASSWORD: tiramisu_pass
      POSTGRES_DB: tiramisu_alpha
    volumes:
      - db_alpha_data:/var/lib/postgresql/data

  env-beta:
    build:
      context: .
      dockerfile: Dockerfile.runtime
    container_name: tiramisu-env-beta
    ports:
      - "8082:80"
      - "2222:22"
    volumes:
      - beta_apps:/var/tiramisu/apps
    environment:
      - ENV_NAME=beta
      - DB_HOST=db-beta
      - DB_USER=tiramisu_user
      - DB_PASSWORD=tiramisu_pass
      - DB_NAME=tiramisu_beta
    depends_on:
      - db-beta

  db-beta:
    image: postgres:16-alpine
    container_name: tiramisu-db-beta
    environment:
      POSTGRES_USER: tiramisu_user
      POSTGRES_PASSWORD: tiramisu_pass
      POSTGRES_DB: tiramisu_beta
    volumes:
      - db_beta_data:/var/lib/postgresql/data

volumes:
  alpha_apps:
  beta_apps:
  db_alpha_data:
  db_beta_data:
)yaml";

constexpr std::string_view DOCKERFILE = R"(
FROM ubuntu:24.04

# Avoid prompts during apt install
ENV DEBIAN_FRONTEND=noninteractive

# Install Nginx, SSH, and basic compilation tools for Caffeine
RUN apt-get update && apt-get install -y \
    nginx \
    openssh-server \
    build-essential \
    gcc \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Configure SSH daemon
RUN mkdir /var/run/sshd
RUN echo 'root:tiramisu_local_dev' | chpasswd
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config

# Set up Tiramisu internal directory paths
RUN mkdir -p /var/run/tiramisu && mkdir -p /var/tiramisu/apps

# Copy Nginx config and startup entrypoint script
COPY nginx.conf /etc/nginx/nginx.conf
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

EXPOSE 80 22

ENTRYPOINT ["/entrypoint.sh"]
)";

// The embedded nginx.conf
constexpr std::string_view NGINX_CONF = R"nginx(
events { worker_connections 1024; }

http {
    include       /etc/nginx/mime.types;
    default_type  application/octet-stream;

    server {
        listen 80;
        server_name localhost;

        location ~ ^/api/(?<project_name>[^/]+)(?<app_path>/.*)$ {
            proxy_set_header X-Tiramisu-Project $project_name;
            proxy_set_header X-Tiramisu-Path $app_path;
            proxy_pass http://unix:/var/run/tiramisu/caffeine.sock;
        }
    }
}
)nginx";

}