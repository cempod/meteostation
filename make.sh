cd nuttx-apps
if [ ! -d "src" ]; then
   ln -s ../src src
fi
cd ../nuttx
if command -v bear &> /dev/null; then
   echo "The project will be built using bear"
   bear -- make -j
else
   echo "If bear is not found, the build will proceed with make only"
   make -j
fi
