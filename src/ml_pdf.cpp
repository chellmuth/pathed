#include "ml_pdf.h"

#include "globals.h"
#include "job.h"
#include "util.h"

#include <assert.h>
#include <cmath>
#include <iostream>
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

void MLPDF::sample(float *phi, float *theta, float *pdf, std::vector<float> &photonBundle) const
{
    float *photonData = photonBundle.data();
    send(m_socket, photonData, sizeof(float) * photonBundle.size(), 0);

    float buffer[3] = {0.f, 0.f, 0.f};
    int bytesRead = recv(m_socket, buffer, sizeof(buffer), 0);

    assert(bytesRead == sizeof(float) * 3);

    *phi = buffer[0] * M_TWO_PI;
    *theta = (1.f - buffer[1]) * (M_PI / 2.f);
    // *theta = buffer[1] * (M_PI / 2.f);
    *pdf = buffer[2];
    // printf("(%f %f %f) (%f %f)\n", *phi, *theta, *pdf, buffer[0], buffer[1]);
}

void MLPDF::estimatePDF(std::vector<float> &radianceLookup, std::vector<float> &photonBundle) const
{
    const int phiSteps = g_job->width();
    const int thetaSteps = g_job->height();
    const int sampleCount = 10000;

    for (int i = 0; i < sampleCount; i++) {
        float phi, theta, pdf;
        sample(&phi, &theta, &pdf, photonBundle);

        int phiIndex = floorf(phi / M_TWO_PI * phiSteps);
        int thetaIndex = floorf(theta / (M_PI / 2.f) * thetaSteps);

        radianceLookup[3 * (thetaIndex * phiSteps + phiIndex) + 0] += 1.f / 100;
        radianceLookup[3 * (thetaIndex * phiSteps + phiIndex) + 1] += 1.f / 100;
        radianceLookup[3 * (thetaIndex * phiSteps + phiIndex) + 2] += 1.f / 100;
    }
}
