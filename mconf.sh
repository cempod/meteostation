cd nuttx-apps
ln -s ../src src
cd ../nuttx 
make distclean
./tools/configure.sh -l ../device/configs/main
cd ../nuttx-apps
rm -rf src
