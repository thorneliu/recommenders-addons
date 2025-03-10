#syntax=docker/dockerfile:1.1.5-experimental
FROM python:3.6 as build_wheel

ARG TF_VERSION=2.5.1
ARG USE_BAZEL_VERSION=3.7.2

RUN pip install --default-timeout=1000 tensorflow-cpu==$TF_VERSION

RUN apt-get update && apt-get install -y sudo rsync cmake
COPY tools/docker/install/install_bazel.sh ./
RUN ./install_bazel.sh $USE_BAZEL_VERSION

COPY requirements.txt ./
RUN pip install -r requirements.txt

COPY tools/install_deps/pytest.txt ./
RUN pip install -r pytest.txt pytest-cov

COPY ./ /recommenders-addons
WORKDIR recommenders-addons
RUN python configure.py
RUN pip install -e ./
RUN --mount=type=cache,id=cache_bazel,target=/root/.cache/bazel \
    bash tools/install_so_files.sh
RUN pytest -v -s -n auto --durations=25 --doctest-modules ./tensorflow_recommenders_addons \
    --cov=tensorflow_recommenders_addons ./tensorflow_recommenders_addons/

RUN bazel build --enable_runfiles build_pip_pkg
RUN bazel-bin/build_pip_pkg artifacts


FROM python:3.6

COPY tools/install_deps/tensorflow-cpu.txt ./
RUN pip install --default-timeout=1000 --upgrade --force-reinstall -r tensorflow-cpu.txt

COPY --from=0 /recommenders-addons/artifacts /artifacts

RUN pip install /artifacts/tensorflow_recommenders_addons-*.whl

# check that we didnd't forget to add a py file to
# The corresponding BUILD file.
# Also test that the wheel works in a fresh environment
RUN python -c "import tensorflow_recommenders_addons"
