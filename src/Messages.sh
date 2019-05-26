#!bin/sh

EXTRACTRC=$KRAFT_HOME/src/extractrc
XGETTEXT=xgettext

if [ -z "$KRAFT_HOME" ]; then
    echo "Need to set KRAFT_HOME"
    exit 1
fi 

podir=$KRAFT_HOME/po
echo "PODir: " $podir
pushd $KRAFT_HOME/src
$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp
$XGETTEXT --kde --language=C++ --from-code=UTF-8 `find . -name \*.cc -o -name \*.cpp -o -name \*.h` -o $podir/kraft.pot
