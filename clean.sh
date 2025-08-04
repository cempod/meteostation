cd nuttx-apps
if [ -d "src" ]; then
    cd ../nuttx
    make distclean
    cd ../nuttx-apps
    rm src
fi