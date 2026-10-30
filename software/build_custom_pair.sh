#!/usr/bin/env bash
set -euo pipefail

# Every applet bundle (less a few), Captain MIDI, no Wave-Edit, 8 presets.
LTO_BUILD_REQUEST='oc-build/1 target=T32_VOR apps=+APPLET_SEQ,+APPLET_Q,+APPLET_MOD,+APPLET_UTIL,+APPLET_WEIRD,+NOWAVE,+NOCVSEQ,+MOAR_PRESETS,+MIDI,+TRIM,+LTO'
# The same without LTO, minus the applets that no longer fit.
NOLTO_BUILD_REQUEST='oc-build/1 target=T32_VOR apps=+APPLET_SEQ,+APPLET_Q,+APPLET_MOD,+APPLET_UTIL,+APPLET_WEIRD,+NOWAVE,+NOCVSEQ,+MOAR_PRESETS,+MIDI,+TRIM,+NOLTOCUTS'
# Experiments can add flags and tag the output, e.g.
#   T32_EXTRA_BUILD_FLAGS='-DSOME_FLAG' VARIANT=test ./build_custom_pair.sh
# The CI "_profile" builds are:
#   T32_EXTRA_BUILD_FLAGS='-DPHZ_PROFILE_APPLETS' VARIANT=profile ./build_custom_pair.sh
T32_EXTRA_BUILD_FLAGS="${T32_EXTRA_BUILD_FLAGS:-}"
VARIANT="${VARIANT:+_${VARIANT}}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
PIO="${REPO_ROOT}/.venv/bin/pio"

if [[ ! -x "${PIO}" ]]; then
  if command -v pio >/dev/null 2>&1; then
    PIO="$(command -v pio)"
  else
    echo "error: pio not found. Activate your venv or install PlatformIO first." >&2
    exit 1
  fi
fi

LTO_CUSTOM_BUILD_FLAGS="$(GH_COMMENT="${LTO_BUILD_REQUEST}" python3 "${SCRIPT_DIR}/res/parse_build_request.py")"
NOLTO_CUSTOM_BUILD_FLAGS="$(GH_COMMENT="${NOLTO_BUILD_REQUEST}" python3 "${SCRIPT_DIR}/res/parse_build_request.py")"
OC_VERSION="$(tail -n 1 "${SCRIPT_DIR}/src/OC_version.h" | tr -d '"')"
SOURCE_SHORT_SHA="$(git -C "${REPO_ROOT}" rev-parse --short HEAD)"
ARTIFACT_DIR="${REPO_ROOT}/artifacts"

mkdir -p "${ARTIFACT_DIR}"

cd "${SCRIPT_DIR}"

outputs=()

build_firmware() {
  local name="$1"
  local flags="$2"
  local output="${ARTIFACT_DIR}/o_C-phazerville-${OC_VERSION}_${name}${VARIANT}-${SOURCE_SHORT_SHA}.hex"

  echo "Building ${name}${VARIANT}"
  export CUSTOM_BUILD_FLAGS="${flags} -DOC_VERSION_SUFFIX=${name}${VARIANT}"
  "${PIO}" run -t clean -e custom
  "${PIO}" run -e custom
  cp .pio/build/custom/*.hex "${output}"
  outputs+=("${output}")
}

build_firmware "T32"           "${LTO_CUSTOM_BUILD_FLAGS} ${T32_EXTRA_BUILD_FLAGS}"
build_firmware "T32_VOR"       "${LTO_CUSTOM_BUILD_FLAGS} ${T32_EXTRA_BUILD_FLAGS} -DVOR"
build_firmware "T32_NoLTO"     "${NOLTO_CUSTOM_BUILD_FLAGS} ${T32_EXTRA_BUILD_FLAGS}"
build_firmware "T32_VOR_NoLTO" "${NOLTO_CUSTOM_BUILD_FLAGS} ${T32_EXTRA_BUILD_FLAGS} -DVOR"

echo "Built artifacts:"
ls -lh "${outputs[@]}"
