import qbs

CppApplication {
    consoleApplication: true
    files: [
        "intelhex.cpp",
        "intelhex.h",
        "intelhex_exception.h",
        "tests/*.cpp",
        "tests/TestData.h",
    ]

    Group {     // Properties for the produced executable
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }

    cpp.cxxLanguageVersion: "c++17"


}
