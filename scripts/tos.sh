#!/bin/sh

if [ X"$1" != X"" ]; then
        cd "$1"
fi

LNG="RUS POL CHI SIN HAN KOR JPN ITA BRA POR FRA DEU ESP USA ENU"

for i in $LNG
do
	echo "Generating Term Of Service for language \"$i\""
	cp -f tos.txt tos_"$i".txt
	cp -f tos.txt tos-unicode_"$i".txt
done
