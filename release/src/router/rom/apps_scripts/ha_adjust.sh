#!/bin/bash

CONTAINER_NAME="homeassistant-asus"
BASE_PATH="/usr/local/lib/python3.13/site-packages/hass_frontend/static/translations"

if [ -z "$(docker ps -q -f name=^/${CONTAINER_NAME}$)" ]; then
    exit 1
fi

# Restore First (avoid infinite revising)
docker exec "$CONTAINER_NAME" /bin/bash -c \
    "sed -i 's/Country\/Region/Country/g' $BASE_PATH/en-*.json" >/dev/null 2>&1

docker exec "$CONTAINER_NAME" /bin/bash -c \
    "sed -i 's/國家或地區/國家/g' $BASE_PATH/zh-Hant-*.json" >/dev/null 2>&1

docker exec "$CONTAINER_NAME" /bin/bash -c \
    "sed -i 's/country or region /country /g' $BASE_PATH/page-onboarding/en-*.json" >/dev/null 2>&1

docker exec "$CONTAINER_NAME" /bin/bash -c \
    "sed -i 's/國家或地區/國家/g' $BASE_PATH/page-onboarding/zh-Hant-*.json" >/dev/null 2>&1

docker exec "$CONTAINER_NAME" /bin/bash -c \
    "sed -i 's/Country\/Region/Country/g' $BASE_PATH/config/en-*.json" >/dev/null 2>&1

docker exec "$CONTAINER_NAME" /bin/bash -c \
    "sed -i 's/國家或地區/國家/g' $BASE_PATH/config/zh-Hant-*.json" >/dev/null 2>&1

sleep 1

# Adjust
docker exec "$CONTAINER_NAME" /bin/bash -c \
    "sed -i 's/Country/Country\/Region/g' $BASE_PATH/en-*.json" >/dev/null 2>&1

docker exec "$CONTAINER_NAME" /bin/bash -c \
    "sed -i 's/國家/國家或地區/g' $BASE_PATH/zh-Hant-*.json" >/dev/null 2>&1

docker exec "$CONTAINER_NAME" /bin/bash -c \
    "sed -i 's/country /country or region /g' $BASE_PATH/page-onboarding/en-*.json" >/dev/null 2>&1

docker exec "$CONTAINER_NAME" /bin/bash -c \
    "sed -i 's/國家/國家或地區/g' $BASE_PATH/page-onboarding/zh-Hant-*.json" >/dev/null 2>&1

docker exec "$CONTAINER_NAME" /bin/bash -c \
    "sed -i 's/Country/Country\/Region/g' $BASE_PATH/config/en-*.json" >/dev/null 2>&1

docker exec "$CONTAINER_NAME" /bin/bash -c \
    "sed -i 's/國家/國家或地區/g' $BASE_PATH/config/zh-Hant-*.json" >/dev/null 2>&1

docker exec "$CONTAINER_NAME" /bin/bash -c "rm -f $BASE_PATH/*.gz $BASE_PATH/*.br" >/dev/null 2>&1
docker exec "$CONTAINER_NAME" /bin/bash -c "rm -f $BASE_PATH/config/*.gz $BASE_PATH/config/*.br" >/dev/null 2>&1
docker exec "$CONTAINER_NAME" /bin/bash -c "rm -f $BASE_PATH/page-onboarding/*.gz $BASE_PATH/page-onboarding/*.br" >/dev/null 2>&1

docker restart homeassistant-asus

sleep 3

