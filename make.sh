cd nuttx-apps
ln -s ../src src
cd ../nuttx
make -j
cd ../nuttx-apps
rm -rf src
