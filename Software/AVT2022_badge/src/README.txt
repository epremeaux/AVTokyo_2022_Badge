Dependency libraries are in .pio\libdeps\...
TFT_eSPI and TFT_eWidget both have example folders. These examples are .ino files.
Create a folder in the src folder named the example name or something similarly descriptive.
Copy the example .ino file to the folder. Rename it main.cppx
To select an example to be compiled, rename the file to main.cpp. make sure all other files in the src folder are named main.cppx (so as to exclude them from compiling.)
Im sure there is a smarter way to do this. :)

Edit main.cpp files:
#include "TFT_eSPI.h"
#include "TFT_eWidget.h"

these libraries are in the local source dir, not the platform.io source. But they may exist in both, causing compile errors.