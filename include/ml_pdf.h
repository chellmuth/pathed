#pragma once

class MLPDF {
public:
    MLPDF();

    bool connectToModel();
    void go();

private:
    int m_socket;
};
