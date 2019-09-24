#include "ml_pdf.h"

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

static const int PORT = 65432;

MLPDF::MLPDF()
{}

bool MLPDF::connectToModel()
{
    struct sockaddr_in serv_addr;

    if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return false;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return false;
    }

    if (connect(m_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return false;
    }

    return true;
}

void MLPDF::go()
{
    float photonBundle[100];
    for (int i = 0; i < 100; i++) {
        photonBundle[i] = i / 99.f;
    }

    send(m_socket, photonBundle, sizeof(photonBundle), 0);
    printf("Hello message sent\n");

    float buffer[3] = {0.f, 0.f, 0.f};
    int valread = recv(m_socket, buffer, sizeof(buffer), 0);

    float x = buffer[0];
    float y = buffer[1];
    float pdf = buffer[2];
    printf("%f %f %f\n", x, y, pdf);
}
