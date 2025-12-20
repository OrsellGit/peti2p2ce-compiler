#include <iostream>

int main( const int argc, char** argv )
{
    for (int i = 0; i < argc; i++ )
        std::cout << argv[i] << std::endl;

    std::cout << "TEST TEST TEST" << std::endl;

    if (true)
        return -1;

    return 0;
}
