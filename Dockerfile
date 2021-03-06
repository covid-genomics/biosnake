FROM python:3.8.8-alpine3.13
#
# Dockerized application
#
# @Covid Genomics 2021

LABEL version='1.0'
LABEL org.opencontainers.image.description='GenXone pilot project'
LABEL org.opencontainers.image.vendor='Covid Genomics'

# Install all basic dependencies as python/c++ toolchain
RUN apk add --no-cache make gcc g++ zeromq zeromq-dev python3-dev libxml2-dev libxslt-dev cmake jpeg-dev
RUN pip3 install --upgrade pip

# Install Poetry
RUN apk add curl
RUN curl -sSL http://raw.githubusercontent.com/python-poetry/poetry/master/get-poetry.py | python3 -

# Install pyzmq
RUN apk add py3-pyzmq

# Install hdf5
RUN apk add --no-cache \
            --allow-untrusted \
            --repository \
             http://dl-3.alpinelinux.org/alpine/edge/testing \
            hdf5 \
            hdf5-dev && \
    apk add --no-cache \
        build-base

WORKDIR /usr/src/biosnake

RUN mv /usr/bin/xxd /usr/bin/xxd_old
RUN apk add xxd

# Install dependencies
COPY . ./
RUN rm -rf dist build > /dev/null 2> /dev/null
ENTRYPOINT $HOME/.poetry/bin/poetry build