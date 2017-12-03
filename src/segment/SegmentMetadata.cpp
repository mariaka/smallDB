
#include "SegmentMetadata.hpp"

namespace simpledb {

    SegmentMetadata::SegmentMetadata(uint64_t segmentId, uint64_t size) : _segmentId(segmentId), _size(size) {
    }

    SegmentMetadata::SegmentMetadata(uint64_t segmentId) : SegmentMetadata(segmentId, 0) {
    }

    SegmentMetadata::SegmentMetadata() : SegmentMetadata(0, 0) {
    }
}
