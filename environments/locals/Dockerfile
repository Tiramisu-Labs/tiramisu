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