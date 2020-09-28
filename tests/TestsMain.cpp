
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"


// strange bug "undefined reference to `WinMain' " in MinGW
#ifdef __MINGW32__
// Standard C/C++ main entry point
int main (int argc, char * argv[]) {
	return Catch::Session().run( argc, argv );
}
#endif
