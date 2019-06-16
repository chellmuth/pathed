#pragma once

#include <mutex>
#include <vector>

class Image {
public:
    Image(int width, int height);

    void set(int row, int col, float r, float g, float b);
    void debug();

    void save();
    void write(char const *filename);

    const std::vector<unsigned char> &data();
    std::mutex &getLock();

private:
    int m_height, m_width;
    std::vector<unsigned char> m_data;
    std::vector<float> m_raw;
    std::mutex m_lock;
};
