/^$/ { next }
/^gcc/ { printf("cc %s\n", $NF) }
/^g\+\+/ { printf("c++ %s\n", $NF) }
