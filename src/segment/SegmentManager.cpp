
#include "SegmentManager.hpp"

namespace simpledb {

    SegmentManager::SegmentManager(std::string path, std::shared_ptr<BufferManager> buffferManager, std::shared_ptr<FileManager> fileManager) : _bufferManager(buffferManager), _fileManager(fileManager), _segmentManagerFile(path + "segments"), _segments(1, std::make_shared<SegmentMetadata>()) {
        std::ifstream in(_segmentManagerFile, std::ifstream::binary);

        if (!in.is_open()) {
            // assume segment file doesn't exist
            persist();
            return;
        }

        {
            // deserialize number of segments
            decltype(_segments)::size_type size;
            in.read(reinterpret_cast<char*> (&size), sizeof (size));

            // deserialize segments
            // TODO: use range constructor of vector to improve performance
            _segments.reserve(size);
            for (decltype(_segments)::size_type i = 0; i < size; ++i) {
                std::unique_ptr<SegmentMetadata> segment(new SegmentMetadata());
                static_assert(sizeof (*segment) == sizeof (SegmentMetadata), "");
                in.read(reinterpret_cast<char*> (&*segment), sizeof (*segment));
                _segments.emplace_back(std::move(segment));
            }
        }
    }

    uint64_t SegmentManager::create() {
        // search next free segment id
        uint64_t segmentId = 0;
        assert(_segments.size() > 0);
        {
            int i = 1;
            for (auto it = ++(_segments.begin()); it != _segments.end(); ++it, ++i) {
                if ((*it)->segmentId() == 0) {
                    segmentId = i;
                    break;
                }
            }
        }

        if (segmentId == 0) { // no free space found => append
            segmentId = _segments.size();
            _fileManager->create(segmentId);
            _segments.push_back(std::make_shared<SegmentMetadata>(segmentId));
        } else { // use found next free segment id
            _fileManager->create(segmentId);
            _segments[segmentId]->setSegmentId(segmentId);
            _segments[segmentId]->setSize();
        }

        persist();

        return segmentId;
    }

    std::shared_ptr<SegmentMetadata> SegmentManager::retrieve(uint64_t segmentId) {
        assert(checkExists(segmentId));
        return _segments[segmentId];
    }

    void SegmentManager::allocate(uint64_t segmentId, uint64_t min, uint64_t max) {
        assert(checkExists(segmentId));

        uint64_t currentSize = _segments[segmentId]->size();

        if (!(currentSize < min)) {
            return;
        }

        uint64_t newSize = currentSize;
        while (newSize < min) {
            // make sure we grow in every iteration of this loop!
            uint64_t tmpSize = newSize * SEGMENT_GROWTH_FACTOR;
            newSize = std::max(newSize + 1, tmpSize);
        }

        _fileManager->truncate(segmentId, _bufferManager->pageSize(), newSize);
        _segments[segmentId]->setSize(newSize);
        persist();
    }

    void SegmentManager::remove(uint64_t segmentId) {
        assert(checkExists(segmentId));

        _fileManager->remove(segmentId);
        _segments[segmentId]->setSegmentId();
        _segments[segmentId]->setSize();

        persist();
    }

    void SegmentManager::persist() {
        std::ofstream out(_segmentManagerFile, std::ofstream::binary | std::ofstream::trunc);

        assert(out.is_open());
        // TODO: error handling

        // serialize number of segments
        auto size = _segments.size();
        out.write(reinterpret_cast<char*> (&size), sizeof (size));

        // serialize segments
        for (std::shared_ptr<SegmentMetadata> const &i : _segments) {
            static_assert(sizeof (*i) == sizeof (SegmentMetadata), "");
            out.write(reinterpret_cast<char*> (&*i), sizeof (*i));
        }
    }

    bool SegmentManager::checkExists(uint64_t segmentId) {
        return (_segments.size() > segmentId && _segments[segmentId].get() && _segments[segmentId]->segmentId() == segmentId);
    }
}