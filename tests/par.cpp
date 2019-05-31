#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
using namespace std;
int main()
{
    int fd = open("1.txt",O_RDONLY);
    char ch;
    cin >> ch;
    int ret = close(fd);
    cout << ret;
    return 0;
}
