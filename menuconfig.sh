cd nuttx-apps
ln -s ../src src
cd ../nuttx
make menuconfig
cd ../nuttx-apps
rm -rf src
