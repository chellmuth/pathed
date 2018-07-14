class Vector3 {
public:
    Vector3(float x, float y, float z);

    int x() const { return m_x; }
    int y() const { return m_y; }
    int z() const { return m_z; }

private:
    float m_x, m_y, m_z;
};
