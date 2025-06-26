if [ ! -d "build" ]; then
    echo "You have to exec configure.sh first!"
    exit
fi

cd nuttx-apps
ln -s ../src external
cd ..

cmake --build build
rm nuttx-apps/external
