
source scripts/install-gyp.sh

function run_gyp(){
    PYTHONPATH="$GYP_DIR:$PYTHONPATH" python $GYP_DIR/gyp_main.py $@
}

export -f run_gyp
