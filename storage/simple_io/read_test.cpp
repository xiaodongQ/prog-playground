#include <iostream>
#include <fcntl.h>   // For open()
#include <unistd.h>  // For read() and close()
#include <sys/stat.h> // For mode_t and O_RDONLY
#include <errno.h>   // For errno
#include <cstring>   // For memset()

int main() {
    const char* filename = "/etc/fstab";
    int fd;
    char buffer[1024];
    ssize_t bytesRead;

    // 打开文件
    if ((fd = open(filename, O_RDONLY)) == -1) {
        std::cerr << "Error opening file: " << strerror(errno) << std::endl;
        return 1;
    }

    // 清空缓冲区
    memset(buffer, 0, sizeof(buffer));

    // 读取文件
    if ((bytesRead = read(fd, buffer, sizeof(buffer) - 1)) == -1) {
        std::cerr << "Error reading from file: " << strerror(errno) << std::endl;
        close(fd);
        return 1;
    }

    // 关闭文件
    if (close(fd) == -1) {
        std::cerr << "Error closing file: " << strerror(errno) << std::endl;
        return 1;
    }

    // 输出文件内容
    std::cout << "Content of /etc/fstab:" << std::endl;
    std::cout << buffer << std::endl;

    return 0;
}
