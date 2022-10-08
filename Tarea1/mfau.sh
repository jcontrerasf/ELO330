#!/bin/bash
#users that appear in last, in order of number
#of connections (from most to less)

last | awk '{print $1}' | head -n -3 | sort -b  | uniq -c | sort -bgr
