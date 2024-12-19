#/bin/zsh

set -e

CONTROLLERS=(
	0x232b
	0x232c
	0x232d
	0x232e
)

for i in ${CONTROLLERS[@]}; do
	./bin/thermaltake_riing_quad -m $1 -v $2 -p $i
done
