
#ifndef SIMPLEDB_SEGMENT_SEGMENTMANAGER_HPP
#define SIMPLEDB_SEGMENT_SEGMENTMANAGER_HPP

#include "SegmentMetadata.hpp"

#include "buffer.hpp"
#include "file.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace simpledb {

    /**
     * Manages concurrent access to creation, deletion and growth of segments.
     */
    class SegmentManager {
    public:
        /**
         * Creates a new segment manager.
         * @param path file path to save metadata info about segments to
         * @param bufferManager the buffer manager
         * @param fileManager the file manager
         */
        SegmentManager(std::string path, std::shared_ptr<BufferManager> bufferManager, std::shared_ptr<FileManager> fileManager);
        ~SegmentManager() = default;

        SegmentManager(const SegmentManager& orig) = delete;
        SegmentManager& operator=(const SegmentManager& orig) = delete;

        /**
         * Creates a new segment on disk. No disk space is allocated.
         * @return the id of the new segment
         */
        uint64_t create();

        /**
         * Retrieves metadata information about a segment.
         * @param segmentId the segment id
         * @return shared_ptr to segment metadata
         */
        std::shared_ptr<SegmentMetadata> retrieve(uint64_t segmentId);

        /**
         * Enlarges a segment to a given size.
         * If necessary new disk space is allocated.
         * @param segmentId the id of the segment to enlarge
         * @param min the minimum size of the segment in buffer frame sizes
         * @param max a hint for the needed segment size in buffer frame sizes
         */
        void allocate(uint64_t segmentId, uint64_t min, uint64_t max);

        /**
         * Deletes a segment from disk.
         * @param segmentId the id of the segment to delete
         */
        void remove(uint64_t segmentId);

    private:
        static constexpr double SEGMENT_GROWTH_FACTOR = 1.25;
        std::shared_ptr<BufferManager> _bufferManager;
        std::shared_ptr<FileManager> _fileManager;
        std::string _segmentManagerFile;
        std::vector<std::shared_ptr<SegmentMetadata>> _segments;

        /**
         * Persists all changes to the segment manager on disk.
         */
        void persist();

        /**
         * Check if a segment exists in the segment manager.
         * @param segmentId the segment id
         * @return true, if the segment exists; false, otherwise
         */
        bool checkExists(uint64_t segmentId);
    };
}

#endif /* SIMPLEDB_SEGMENT_SEGMENTMANAGER_HPP */
