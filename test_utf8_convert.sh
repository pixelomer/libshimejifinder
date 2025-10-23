#!/usr/bin/env bash

echo "==> Building tests..."
echo

ret=0

for test in tests/utf8_convert/*/; do
    pushd "$test" 2>/dev/null >&2
    echo -n "Building $(basename "${test}") backend... "
    status="done"
    (cmake -Bbuild && make -Cbuild -j"$(nproc)") 2>/dev/null >&2 || { ret=1; status="failed"; }
    echo "${status}"
    popd 2>/dev/null >&2
done

for test in tests/utf8_convert/*/; do
    echo
    echo "==> Testing backend: $(basename "${test}")"
    echo
    "${test}/build/$(basename "${test}")_test" || ret=1
done

echo
if [[ "${ret}" == 0 ]]; then
    echo "All tests passed."
else
    echo "Some tests failed."
fi
[[ "${ret}" == 0 ]]
