class Point3 {
public:
    Point3(float x, float y, float z);

    int x() const { return m_x; }
    int y() const { return m_y; }
    int z() const { return m_z; }

private:
    float m_x, m_y, m_z;
};
