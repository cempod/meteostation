cd nuttx-apps
ln -s ../src external
cd ..
cmake --build build -t menuconfig
rm nuttx-apps/external
