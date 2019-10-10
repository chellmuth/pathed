#include "ml_pdf.h"

#include "globals.h"
#include "job.h"
#include "util.h"

#include "omp.h"

#include <assert.h>
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

static const int PORT = 65432;
static const int THREAD_COUNT = 4;

MLPDFPool::MLPDFPool()
{}

bool MLPDFPool::connectToModel()
{
    for (int i = 0; i < THREAD_COUNT; i++) {
        auto pdf = MLPDF();
        if (!pdf.connectToModel(i)) {
            return false;
        }
        m_pdfs.push_back(pdf);
    }
    return true;
}

void MLPDFPool::sample(float *x, float *y, float *pdf, std::vector<float> &photonBundle) const
{
    const int threadID = omp_get_thread_num();
    auto &localPDF = m_pdfs[threadID];

    localPDF.sample(x, y, pdf, photonBundle);
}


MLPDF::MLPDF()
{}

bool MLPDF::connectToModel(int portOffset)
{
    struct sockaddr_in serv_addr;

    if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return false;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT + portOffset);

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
    *pdf = buffer[2] / sinf(*theta) / (M_TWO_PI * M_PI / 2.f);

    if (*phi < 0 || *phi > M_TWO_PI || *theta < 0 || *theta > (M_PI/2.f) || *pdf < 0) {
        std::cout << "UHOH: " << *phi << " " << *theta << " " << *pdf
                  << " " << buffer[0] << " " << buffer[1] << " " << buffer[2]
                  << std::endl;

        sample(phi, theta, pdf, photonBundle);
    }
    // printf("(%f %f %f) (%f %f)\n", *phi, *theta, *pdf, buffer[0], buffer[1]);
}

void MLPDF::batchSample(
    int count,
    std::vector<float> &phis,
    std::vector<float> &thetas,
    std::vector<float> &pdfs,
    std::vector<float> &photonBundles
) const {
    int hello[] = { 0, count };
    send(m_socket, &hello, sizeof(int) * 2, 0);

    float *photonData = photonBundles.data();
    send(m_socket, photonData, sizeof(float) * photonBundles.size(), 0);

    float buffer[3 * count];
    int bytesRead = recv(m_socket, buffer, sizeof(buffer), MSG_WAITALL);
    assert(bytesRead == 3 * count);

    for (int i = 0; i < count; i++) {
        int offset = i * 3;
        phis[i] = buffer[offset + 0] * M_TWO_PI;
        const float theta = (1.f - buffer[offset + 1]) * (M_PI / 2.f);
        thetas[i] = theta;
        pdfs[i] = buffer[offset + 2] / sinf(theta) / (M_TWO_PI * M_PI / 2.f);
    }
}

void MLPDF::batchEval(
    int count,
    std::vector<float> &pdfs,
    std::vector<float> &photonBundles
) const {
    int hello[] = { 1, count };
    send(m_socket, &hello, sizeof(int) * 2, 0);

    float *photonData = photonBundles.data();
    send(m_socket, photonData, sizeof(float) * photonBundles.size(), 0);

    float buffer[count];
    int bytesRead = recv(m_socket, buffer, sizeof(buffer), MSG_WAITALL);
    assert(bytesRead == 4 * count);

    const int rows = (int)sqrtf(count);
    const int cols = (int)sqrtf(count);

    for (int i = 0; i < count; i++) {
        int row = (int)floorf(i / cols);
        int col = i % cols;

        const float theta = M_PI / 2.f * (row + 0.5f) / rows;
        pdfs[i] = buffer[i] / sinf(theta) / (M_TWO_PI * M_PI / 2.f);
    }
}
