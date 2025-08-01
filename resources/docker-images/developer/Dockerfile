# syntax=docker/dockerfile:1.4
# Keyman is copyright (C) SIL Global. MIT License.
#
# ARGS used in this file:
# - ARG BASE_VERSION=default
# - ARG REQUIRED_EMSCRIPTEN_VERSION=unset

ARG BASE_VERSION=default
FROM keymanapp/keyman-base-ci:${BASE_VERSION}

LABEL org.opencontainers.image.authors="SIL Global."
LABEL org.opencontainers.image.url="https://github.com/keymanapp/keyman.git"
LABEL org.opencontainers.image.title="Keyman Developer Build Image"

USER root
RUN apt-get update && \
  apt-get install -qy devscripts jq meson

# Pre-install emscripten
USER build
ARG REQUIRED_EMSCRIPTEN_VERSION=unset
RUN echo "Installing emscripten version ${REQUIRED_EMSCRIPTEN_VERSION}" && \
  export EMSDK_KEEP_DOWNLOADS=1 && \
  cd /home/build/ && \
  git clone https://github.com/emscripten-core/emsdk.git && \
  cd emsdk && \
  ./emsdk install ${REQUIRED_EMSCRIPTEN_VERSION} && \
  ./emsdk activate ${REQUIRED_EMSCRIPTEN_VERSION}
USER root
RUN echo "export EMSCRIPTEN_BASE=/home/build/emsdk/upstream/emscripten" >> /usr/bin/bashwrapper && \
  echo "export KEYMAN_USE_EMSDK=1" >> /usr/bin/bashwrapper

# Finish bashwrapper script and adjust permissions
RUN <<EOF cat >> /usr/bin/bashwrapper

if [[ "\$@" =~ test ]] && [ -f /usr/bin/run-tests.sh ]; then
  /usr/bin/run-tests.sh "\${@:-bash}"
else
  "\${@:-bash}"
fi
EOF

# now, switch to build user
USER build

# Pre-install node
RUN export NVM_DIR="/home/build/.nvm" && \
  . /home/build/.nvm/nvm.sh && \
  cd /home/build/emsdk/upstream/emscripten && \
  npm install

VOLUME /home/build/build
WORKDIR /home/build/build

ENTRYPOINT [ "/usr/bin/bashwrapper" ]
