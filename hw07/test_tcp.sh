#!/bin/bash

exec 5<>/dev/tcp/127.0.0.1/2222
cat <&5

exit 0