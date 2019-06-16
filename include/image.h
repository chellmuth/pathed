#pragma once

#include <mutex>
#include <vector>

class Image {
public:
    Image(int width, int height);

    void set(int row, int col, float r, float g, float b);
    void debug();

    void save(char const *filestem);
    void write(char const *filename);

    const std::vector<unsigned char> &data();
    std::mutex &getLock();

    void setSpp(int spp) { m_spp = spp; }

private:
    int m_height, m_width;
    int m_spp;
    std::vector<unsigned char> m_data;
    std::vector<float> m_raw;
    std::mutex m_lock;
};
