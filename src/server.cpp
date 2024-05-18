#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iomanip>

struct DNSHeader {
    uint16_t ID;
    uint16_t flags;
    uint16_t QDCOUNT;
    uint16_t ANCOUNT;
    uint16_t NSCOUNT;
    uint16_t ARCOUNT;
    DNSHeader(int16_t ID, bool QR, bool OPCODE[4], bool AA, bool TC, bool RD, bool RA, bool Z[3], bool RCODE[4], int16_t QDCOUNT,
        int16_t ANCOUNT, int16_t NSCOUNT, int16_t ARCOUNT)
        : ID(htons(ID)), QDCOUNT(htons(QDCOUNT)), ANCOUNT(htons(ANCOUNT)), NSCOUNT(htons(NSCOUNT)), ARCOUNT(htons(ARCOUNT)) {
        setflags(QR, OPCODE, AA, TC, RD, RA, Z, RCODE);
    }
    inline void setflags(bool QR, bool OPCODE[4], bool AA, bool TC, bool RD, bool RA, bool Z[3], bool RCODE[4])
    {
        flags = 0;
        flags |= htons((QR << 15));
        for (int i = 0; i < 4; ++i)
            flags |= htons((OPCODE[i] << (14 - i)));
        flags |= htons((AA << 10));
        flags |= htons((TC << 9));
        flags |= htons((RD << 8));
        flags |= htons((RA << 7));
        for (int i = 0; i < 3; ++i)
            flags |= htons((Z[i] << (6 - i)));
        for (int i = 0; i < 4; ++i)
            flags |= htons((RCODE[i] << (3 - i)));
    };
};

int main() {
    // Disable output buffering
    setbuf(stdout, NULL);

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cout << "Logs from your program will appear here!" << std::endl;

    // Uncomment this block to pass the first stage
    int udpSocket;
    struct sockaddr_in clientAddress;

    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        std::cerr << "Socket creation failed: " << strerror(errno) << "..." << std::endl;
        return 1;
    }

    // Since the tester restarts your program quite often, setting REUSE_PORT
    // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "SO_REUSEPORT failed: " << strerror(errno) << std::endl;
        return 1;
    }

    sockaddr_in serv_addr = { .sin_family = AF_INET,
                              .sin_port = htons(2053),
                              .sin_addr = { htonl(INADDR_ANY) },
    };

    if (bind(udpSocket, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) != 0) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        return 1;
    }

    int bytesRead;
    char buffer[512];
    socklen_t clientAddrLen = sizeof(clientAddress);

    while (true) {
        // Receive data
        bytesRead = recvfrom(udpSocket, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddrLen);
        if (bytesRead == -1) {
            perror("Error receiving data");
            break;
        }

        buffer[bytesRead] = '\0';
        std::cout << "Received " << bytesRead << " bytes: " << buffer << std::endl;
        bool OPCODE[4] = { false,false,false,false };
        bool Z[3] = { false,false,false };
        bool RCODE[4] = { false,false,false,false };
        DNSHeader response = DNSHeader(1234, true, OPCODE, false, false, false, false, Z, RCODE, 0, 0, 0, 0);
        // Send response
        if (sendto(udpSocket, &response, sizeof(DNSHeader), 0, reinterpret_cast<struct sockaddr*>(&clientAddress), sizeof(clientAddress)) == -1) {
            perror("Failed to send response");
        }
    }

    close(udpSocket);

    return 0;
}
