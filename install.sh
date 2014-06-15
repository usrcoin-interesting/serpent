#!/bin/bash
rm -f Makefile
if [[ "$OSTYPE" == 'darwin' || "$OSTYPE" == 'darwin13' ]]; then
    echo 'PLATFORM_OPTS = ' >> Makefile
    cat Makefile.in | tail -n `expr $(cat Makefile.in | wc -l) - 1` >> Makefile
else
    cat Makefile.in >> Makefile
fi
if [[ $1 == 'clean' ]]; then
    make clean
else
    make
    sudo make install
fi