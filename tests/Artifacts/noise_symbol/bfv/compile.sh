#!/bin/bash

HEIR_OPT="bazel run //tools:heir-opt -- "
HEIR_TRANSLATE="bazel run //tools:heir-translate -- "

NOISE_MODEL=${NOISE_MODEL:-"bfv-noise-symbol"}

declare -a HEIR_OPT_FLAGS
HEIR_OPT_FLAGS+=("--wrap-generic")
HEIR_OPT_FLAGS+=("--annotate-module=backend=openfhe")
HEIR_OPT_FLAGS+=("--secret-insert-mgmt-bfv")
# TODO: fix logN for Openfhe
HEIR_OPT_FLAGS+=("--generate-param-bfv")
HEIR_OPT_FLAGS+=("--annotate-module=backend=lattigo")
HEIR_OPT_FLAGS+=("--validate-noise=annotate-noise-bound=true model=${NOISE_MODEL}")
HEIR_OPT_FLAGS+=("--secret-distribute-generic")
HEIR_OPT_FLAGS+=("--secret-to-bgv")
HEIR_OPT_FLAGS+=("--lwe-add-client-interface")
HEIR_OPT_FLAGS+=("--scheme-to-openfhe=insert-debug-handler-calls=true")

declare -a HEIR_TRANSLATE_FLAGS
HEIR_TRANSLATE_FLAGS+=("--emit-openfhe-pke")
HEIR_TRANSLATE_FLAGS+=("--openfhe-include-type=embedded")

SRC=`realpath $1`

OUT=`realpath $2`

set -x

${HEIR_OPT} ${SRC} "${HEIR_OPT_FLAGS[@]}" | ${HEIR_TRANSLATE} "${HEIR_TRANSLATE_FLAGS[@]}" -o ${OUT}
