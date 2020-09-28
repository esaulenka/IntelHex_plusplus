// strange bug "undefined reference to `WinMain' " in MinGW
#ifdef __MINGW32__
	#define DO_NOT_USE_WMAIN
#endif

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"


