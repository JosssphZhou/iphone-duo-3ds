#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd "$(dirname "$0")/.." && pwd)"
native_root="${DUO_NATIVE_ROOT:-$repo_root/iphone-duo-3ds-native}"
test_dir="$(mktemp -d "${TMPDIR:-/tmp}/duo-host.XXXXXX")"
trap 'rm -rf "$test_dir"' EXIT
compiler="${CC:-cc}"
flags=(-D_POSIX_C_SOURCE=200809L -std=c11 -Wall -Wextra -Werror -I "$native_root/include")
if [[ "${SANITIZE:-0}" == 1 ]]; then
  flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer)
fi
run_test() {
  local name="$1"
  shift
  local source_files=()
  for unit in "$@"; do source_files+=("$native_root/source/$unit.c"); done
  "$compiler" "${flags[@]}" "$native_root/tests/${name}_test.c" "${source_files[@]}" -lm -o "$test_dir/$name"
  "$test_dir/$name"
}
run_test model model geometry layout_storage
run_test interaction interaction geometry
run_test orientation orientation
run_test lockscreen lockscreen
run_test showcase showcase
run_test widget_remove widget_remove model geometry
run_test language language
