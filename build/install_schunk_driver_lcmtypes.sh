mkdir -p include/lcmtypes/schunk_driver
cp schunk-driver-prefix/src/schunk-driver-build/lcmgen/lcmtypes/*.hpp include/lcmtypes
cp schunk-driver-prefix/src/schunk-driver-build/lcmgen/lcmtypes/*.h include/lcmtypes
cp schunk-driver-prefix/src/schunk-driver-build/lcmgen/lcmtypes/schunk_driver/*.hpp include/lcmtypes/schunk_driver
cp schunk-driver-prefix/src/schunk-driver-build/lib/*.a lib
cp schunk-driver-prefix/src/schunk-driver-build/*.jar share/java
