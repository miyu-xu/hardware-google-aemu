#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <output_file>" << std::endl;
        return 1;
    }

    std::ofstream outfile(argv[1]);
    if (!outfile) {
        std::cerr << "Error opening file: " << argv[1] << std::endl;
        return 1;
    }

    outfile << "#define MY_PKGCONFIG_GREETING \"hello, from my_pkgconfig_prog\"" << std::endl;
    outfile.close();

    return 0;
}

