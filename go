#!/bin/bash
set -eux

d='drivers'
s='scripts/coccinelle/free/free_skb.cocci'
modes=(report org context patch)

cocci_opts=( \
	COCCI="$s" \
	DEBUG_FILE=err.log \
	J=29 \
	M="$d" \
	V=1 \
	)

set -o pipefail

for m in "${modes[@]}"; do
	rm -vf err.log

	if ! make -l42 coccicheck "${cocci_opts[@]}" MODE="$m"; then
		cat err.log
		exit 1
	fi |& tee "$m".log
done
