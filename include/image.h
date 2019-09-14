#pragma once

#include <mutex>
#include <string>
#include <vector>

class Image {
public:
    Image(int width, int height);

    void set(int row, int col, float r, float g, float b);
    void debug();

    void save(const std::string &filestem);
    void write(const std::string &filename);

    const std::vector<unsigned char> &data();
    std::mutex &getLock();

    void setSpp(int spp) { m_spp = spp; }

private:
    std::string pathFromFilename(const std::string &filename);

    int m_height, m_width;
    int m_spp;
    std::vector<unsigned char> m_data;
    std::vector<float> m_raw;
    std::mutex m_lock;
};
