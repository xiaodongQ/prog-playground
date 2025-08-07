#include <iostream>
#include <cstdint>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <ctime>

// 此处设置pack后，会按1字节对齐，忽略结构体的对齐补齐
#pragma pack(push, 1)
struct Header {
    // 2字节字符数组
    char magic[2];
    uint16_t version;
    uint16_t command;
    uint32_t bodyLen;
    uint32_t checksum;
};

struct Body {
    uint32_t seqNum;
    uint64_t timestamp;
    char data[4];
};
#pragma pack(pop)

bool recvAll(int sock, void* buf, size_t len) {
    char* ptr = (char*)buf;
    while (len > 0) {
        ssize_t rc = recv(sock, ptr, len, 0);
        std::cout << "len:" << len << ", rc:" << rc << std::endl;
        if (rc <= 0) {
            std::cerr << "接收失败: " << (rc == 0 ? "连接关闭" : strerror(errno)) << std::endl;
            return false;
        }
        ptr += rc;
        len -= rc;
    }
    return true;
}

uint32_t calculateChecksum(const char* data, uint32_t len) {
    uint32_t sum = 0;
    for (uint32_t i = 0; i < len; ++i) {
        sum += (uint8_t)data[i];
    }
    return sum;
}

void printPacket(const Header& header, const Body& body, const char* raw, size_t totalLen) {
    std::cout << "\nReceived Packet:\n";
    std::cout << "Header:\n";
    std::cout << "Magic: ";
    for (int i = 0; i < 2; ++i) {
        printf("%c(0x%02X) ", header.magic[i], (uint8_t)header.magic[i]);
    }
    std::cout << std::endl;
    // 已经转换过本地字节序了，此处不用再转换
    std::cout << "  Version:  0x" << std::hex << header.version << "\n";
    std::cout << "  Command:  0x" << std::hex << header.command << "\n";
    std::cout << "  BodyLen:  " << std::dec << header.bodyLen << "\n";
    std::cout << "  Checksum: 0x" << std::hex << header.checksum << "\n";

    std::cout << "\nBody:\n";
    std::cout << "  SeqNum:   0x" << std::hex << body.seqNum << "\n";
    std::cout << "  Timestamp:0x" << std::hex << body.timestamp << "\n";
    std::cout << "  Data:     '";
    for (int i = 0; i < 4; ++i) {
        if (isprint(body.data[i])) std::cout << body.data[i];
        else std::cout << "\\x" << std::hex << (int)(uint8_t)body.data[i];
    }
    std::cout << "'\n";

    std::cout << "\nRaw Bytes (" << totalLen << " bytes):\n";
    for (size_t i = 0; i < totalLen; i += 8) {
        size_t end = std::min(i + 8, totalLen);
        printf("  [%04zX] ", i);
        for (size_t j = i; j < end; ++j) {
            printf("%02X ", (uint8_t)raw[j]);
        }
        std::cout << "\n";
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "socket() failed: " << strerror(errno) << std::endl;
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "bind() failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        std::cerr << "listen() failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return 1;
    }

    std::cout << "Server listening on port 8080..." << std::endl;

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            std::cerr << "accept() failed: " << strerror(errno) << std::endl;
            continue;
        }

        // 接收协议头
        Header header;
        std::cout << "size Header:" << sizeof(header) << std::endl;
        if (!recvAll(client_fd, &header, sizeof(header))) {
            std::cerr << "recv header failed" << std::endl;
            close(client_fd);
            continue;
        }

        // 验证Magic
        if (header.magic[0] != 'A' || header.magic[1] != 'B') {
            std::cerr << "Invalid magic: " 
                      << header.magic[0] << header.magic[1] << std::endl;
            close(client_fd);
            continue;
        }

        // 转换成本地字节序
        header.version = ntohs(header.version);
        header.command = ntohs(header.command);
        header.bodyLen = ntohl(header.bodyLen);
        header.checksum = ntohl(header.checksum);

        // 接收协议体
        std::cout << "bodyLen:" << header.bodyLen << std::endl;
        char* bodyData = new char[header.bodyLen];
        if (!recvAll(client_fd, bodyData, header.bodyLen)) {
            std::cerr << "recv body failed" << std::endl;
            delete[] bodyData;
            close(client_fd);
            continue;
        }

        // 验证校验和（基于大端字节序数据计算，两端保持一致）
        uint32_t expectedChecksum = calculateChecksum(bodyData, header.bodyLen);
        if (header.checksum != expectedChecksum) {
            std::cerr << "checksum mismatch (expected: 0x" 
                      << std::hex << expectedChecksum 
                      << ", got: 0x" << header.checksum 
                      << ")" << std::endl;
            delete[] bodyData;
            close(client_fd);
            continue;
        }

        // 反序列化协议体
        Body body;
        if (header.bodyLen >= sizeof(body)) {
            memcpy(&body, bodyData, sizeof(body));
            // 转换成本地字节序
            body.seqNum = ntohl(body.seqNum);
            body.timestamp = be64toh(body.timestamp);
        } else {
            std::cerr << "Body too small: " << header.bodyLen 
                      << " < " << sizeof(body) << std::endl;
            close(client_fd);
            continue;
        }

        // 组合原始报文用于打印
        char* fullPacket = new char[sizeof(header) + header.bodyLen];
        memcpy(fullPacket, &header, sizeof(header));
        memcpy(fullPacket + sizeof(header), bodyData, header.bodyLen);

        printPacket(header, body, fullPacket, sizeof(header) + header.bodyLen);

        delete[] bodyData;
        delete[] fullPacket;
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
