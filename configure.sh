cd nuttx-apps
ln -s ../src external
cd ..
cmake nuttx -B build -DBOARD_CONFIG=../device/configs/main -GNinja
rm nuttx-apps/external
