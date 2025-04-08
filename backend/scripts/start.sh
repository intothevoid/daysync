#!/bin/bash

docker run --rm -p5173:5173 --env-file api.env daysync:latest