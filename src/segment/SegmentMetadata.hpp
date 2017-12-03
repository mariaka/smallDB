
#ifndef SIMPLEDB_SEGMENT_SEGMENTMETADATA_HPP
#define SIMPLEDB_SEGMENT_SEGMENTMETADATA_HPP

#include <cstdint>

namespace simpledb {

    /**
     * Holds all segment metadata info that is serialized to disk.
     */
    class SegmentMetadata {
    public:
        /**
         * Creates a new segment metadata object.
         * @param segmentId the segment id
         * @param size the size of the segment in number of pages
         */
        SegmentMetadata(uint64_t segmentId, uint64_t size);

        /**
         * Creates a new segment metdata object with zero size.
         * @param segmentId the segment id
         */
        explicit SegmentMetadata(uint64_t segmentId);

        /**
         * Creates a new segment metadata object with invalid segment id and zero size.
         */
        SegmentMetadata();

        ~SegmentMetadata() = default;

        SegmentMetadata(const SegmentMetadata& orig) = delete;
        SegmentMetadata& operator=(const SegmentMetadata& orig) = delete;

        /**
         * @return the segment id
         */
        uint64_t segmentId() {
            return _segmentId;
        }

        /**
         * Sets the segment id.
         * @param segmentId the segment id
         */
        void setSegmentId(uint64_t segmentId = 0) {
            _segmentId = segmentId;
        }

        /**
         * @return the size of the segment in number of pages
         */
        uint64_t size() {
            return _size;
        }

        /**
         * Sets the size of the segment.
         * @param size the size of the segment in number of pages
         */
        void setSize(uint64_t size = 0) {
            _size = size;
        }

    private:
        uint64_t _segmentId; // segment id
        uint64_t _size; // size in page sizes
    };
}

#endif /* SIMPLEDB_SEGMENT_SEGMENTMETADATA_HPP */
