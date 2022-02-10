FROM ubuntu

LABEL Author_name="Chadi LAOULAOU" Author_email="chadi.laoulaou@abtasty.com"

ENV TZ=Europe/London \
    DEBIAN_FRONTEND=noninteractive

# Update aptitude with new repo
RUN apt-get update

RUN apt-get update --fix-missing

RUN apt-get upgrade -y

# Install software 
RUN apt-get install -y --allow-downgrades curl build-essential wget libc-dev libunwind-dev make gcc libc6=2.31-0ubuntu9.2 zlib1g=1:1.2.11.dfsg-2ubuntu1.2 liblzma-dev liblzma5=5.2.4-1ubuntu1 libpcre3 libpcre3-dev libpcre++-dev zlib1g-dev libbz2-dev libxslt1-dev libxml2-dev libgeoip-dev libgoogle-perftools-dev libperl-dev libssl-dev libcurl4-openssl-dev libatomic-ops-dev nginx=1.18.*

RUN mkdir work_directory

WORKDIR /work_directory

# Download nginx-1.18 package from source
RUN wget https://nginx.org/download/nginx-1.18.0.tar.gz

COPY . .

RUN chmod +x script.sh

RUN mkdir -p /usr/local/nginx/sbin && cp libflagship.so /usr/local/nginx/sbin/

# Extract the tar
RUN tar zxf nginx-1.18.0.tar.gz

RUN cd nginx-1.18.0 && ./configure

RUN cd nginx-1.18.0 && make && make install