make -f Maketracker
gbmod=$2
if [ "$1" = "camera" ]
then
    build/examples/cvtest.bin http://root:xgwangpj@137.189.35.204:10181/mjpg/video.mjpg $gbmod
fi
build/examples/cvtest.bin "$1" $gbmod
#build/examples/cvtest.bin ~/Downloads/Helicopter_DivXHT_ASP.divx
#build/examples/cvtest.bin http://root:xgwangpj@137.189.35.204:10184/mjpg/video.mjpg
#build/examples/utest.bin