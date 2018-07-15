#pragma once

class Image {
public:
    Image(int width, int height);
    ~Image();

    void set(int row, int col, float r, float g, float b);
    void debug();
    void write(char const *filename);

private:
    int m_height, m_width;
    unsigned char *m_data;
};
