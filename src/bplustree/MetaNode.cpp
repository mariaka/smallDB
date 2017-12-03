
#include "MetaNode.hpp"

namespace simpledb {

    void MetaNode::init(uint64_t nextFreePageId) {
        _nextFreePageId = nextFreePageId;
    }

    uint64_t MetaNode::nextFreePageId() {
        return _nextFreePageId++;
    }
}
