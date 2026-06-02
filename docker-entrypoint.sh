#!/bin/sh
set -eu

mkdir -p /coredumps
chmod 1777 /coredumps

ulimit -c unlimited

if [ -w /proc/sys/kernel/core_pattern ]; then
  echo '/coredumps/core.%e.%p.%t' > /proc/sys/kernel/core_pattern
else
  echo "warning: cannot write /proc/sys/kernel/core_pattern; core dumps may follow host settings" >&2
fi

exec "$@"
