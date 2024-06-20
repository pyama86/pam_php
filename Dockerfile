FROM ubuntu:jammy
MAINTAINER pyama86 <www.kazu.com@gmail.com>
ENV LIBDIR /usr/lib
RUN apt update -qqy && \
    DEBIAN_FRONTEND=noninteractive apt install -qqy sudo \
    curl \
    libpam0g-dev \
    build-essential \
    php8.1-dev \
    vim \
    libphp8.1-embed
RUN useradd -m -s /bin/bash seven && useradd -m -s /bin/bash eight
RUN sed -i '1s#^#auth sufficient pam_php.so /work/phpconfuk-2024.php\n#' /etc/pam.d/common-auth
RUN rm -rf /etc/php/8.1/embed/conf.d/*
WORKDIR /work
