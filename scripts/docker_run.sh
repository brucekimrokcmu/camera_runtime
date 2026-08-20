#!/usr/bin/env bash
set -e

IMAGE_NAME="camera_runtime_dev:latest"
CONTAINER_NAME="camera_runtime_container"
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

FORCE_BUILD=false

# Parse flags (e.g., ./docker_run.sh --build)
while [[ $# -gt 0 ]]; do
  case $1 in
  -b | --build)
    FORCE_BUILD=true
    shift
    ;;
  *)
    shift
    ;;
  esac
done

# Check if image exists OR if force build flag was passed
if [ "${FORCE_BUILD}" = true ] || ! docker image inspect "${IMAGE_NAME}" >/dev/null 2>&1; then
  echo "==> Building Docker image: ${IMAGE_NAME}..."
  docker build -f "${PROJECT_DIR}/docker/Dockerfile" -t "${IMAGE_NAME}" "${PROJECT_DIR}"
else
  echo "==> Image '${IMAGE_NAME}' already exists. Skipping build (use --build to rebuild)."
fi

# Remove pre-existing container if present
if docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
  echo "==> Removing existing container '${CONTAINER_NAME}'..."
  docker rm -f "${CONTAINER_NAME}" >/dev/null 2>&1
fi

echo "==> Launching development container..."
xhost +local:docker >/dev/null 2>&1 || true

docker run -it --rm \
  --name "${CONTAINER_NAME}" \
  --privileged \
  -v /dev:/dev \
  -v "${PROJECT_DIR}:${PROJECT_DIR}" \
  -w "${PROJECT_DIR}" \
  --group-add video \
  --ipc=host \
  -e DISPLAY="${DISPLAY}" \
  -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
  "${IMAGE_NAME}"
