#!/bin/bash

docker run -d --name daysync -p5173:5173 --env-file api.env --restart unless-stopped daysync:latest