#pragma once

namespace MIS {
    inline float balanceWeight(int n1, int n2, float pdf1, float pdf2)
    {
        return (n1 * pdf1) / (n1 * pdf1 + n2 * pdf2);
    }

    inline float powerWeight(int n1, int n2, float pdf1, float pdf2)
    {
        const float effectivePDF1 = n1 * pdf1;
        const float effectivePDF2 = n2 * pdf2;

        return (effectivePDF1 * effectivePDF1)
            / (effectivePDF1 * effectivePDF1 + effectivePDF2 * effectivePDF2);
    }
};
