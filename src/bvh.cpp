#include "bvh.h"
#include <algorithm>
#include <limits>

BVH::BVH(const std::vector<Data>& inDataArray) : dataArray(inDataArray) {
    buildBVH();
}

void BVH::buildBVH() {
    nodes.reserve(2 * dataArray.size() - 1);
    recursiveBuild(0, static_cast<int>(dataArray.size()), 0);
}

int BVH::recursiveBuild(int start, int end, int depth) {
    int nodeIndex = static_cast<int>(nodes.size());
    nodes.emplace_back();
    BVHNode& node = nodes.back();

    // ノードの境界ボックスを計算
    glm::vec3 min(std::numeric_limits<float>::max());
    glm::vec3 max(std::numeric_limits<float>::lowest());
    for (int i = start; i < end; ++i) {
        const Data& data = dataArray[i];
        min = glm::min(min, glm::min(glm::vec3(data.v0), glm::min(glm::vec3(data.v1), glm::vec3(data.v2))));
        max = glm::max(max, glm::max(glm::vec3(data.v0), glm::max(glm::vec3(data.v1), glm::vec3(data.v2))));
    }
    node.min = glm::vec4(min, 0.0f);
    node.max = glm::vec4(max, 0.0f);

    int numData = end - start;
    if (numData <= 4 || depth > 20) {
        // 葉ノード
        node.data = glm::ivec4(-1, -1, start, numData);
    } else {
        // 内部ノード
        int axis = depth % 3;
        int mid = (start + end) / 2;

        // データを軸に沿ってソート
        std::nth_element(dataArray.begin() + start,
                         dataArray.begin() + mid,
                         dataArray.begin() + end,
                         [&](const Data& a, const Data& b) {
                             return calculateCentroid(a)[axis] < calculateCentroid(b)[axis];
                         });

        int leftChild = recursiveBuild(start, mid, depth + 1);
        int rightChild = recursiveBuild(mid, end, depth + 1);
        node.data = glm::ivec4(leftChild, rightChild, -1, -1);
    }

    return nodeIndex;
}

glm::vec3 BVH::calculateCentroid(const Data& data) const {
    return (glm::vec3(data.v0) + glm::vec3(data.v1) + glm::vec3(data.v2)) / 3.0f;
}