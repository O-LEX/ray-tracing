#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "util.h"

struct BVHNode {
    glm::vec4 min;          // 16 bytes
    glm::vec4 max;          // 16 bytes
    union {
        struct {
            int left;
            int right;
            int dataOffset;
            int dataCount;
        };
        glm::ivec4 data;    // 16 bytes
    };
};

class BVH {
public:
    BVH(const std::vector<Data>& dataArray);
    ~BVH() = default;

    const std::vector<BVHNode>& getNodes() const { return nodes; }
    const std::vector<Data>& getDataArray() const { return dataArray; }

private:
    void buildBVH();
    int recursiveBuild(int start, int end, int depth);
    glm::vec3 calculateCentroid(const Data& data) const;
    
    std::vector<BVHNode> nodes;
    std::vector<Data> dataArray;
};