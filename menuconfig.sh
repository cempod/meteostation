cd nuttx-apps
if [ ! -d "src" ]; then
   ln -s ../src src
fi
cd ../nuttx
make menuconfig
