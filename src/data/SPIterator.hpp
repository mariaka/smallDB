
#ifndef SIMPLEDB_DATA_SPITERATOR_HPP
#define SIMPLEDB_DATA_SPITERATOR_HPP

#include "Record.hpp"
#include "SPPage.hpp"
#include "SPSlot.hpp"

#include "buffer.hpp"
#include "segment.hpp"

#include <cassert>
#include <cstdint>
#include <memory>

namespace simpledb {

    class SPIterator {
    public:
        SPIterator();
        SPIterator(uint64_t segmentId, std::shared_ptr<SegmentManager> segmentManager, std::shared_ptr<BufferManager> bufferManager);
        ~SPIterator();

        SPIterator(const SPIterator& orig) = delete;
        SPIterator& operator=(const SPIterator& orig) = delete;

        bool isValid() const;
        Record operator*() const;
        SPIterator& operator++();

    private:
        uint64_t _segmentId;
        uint64_t _segmentSize;
        uint64_t _page;
        int64_t _slot;

        std::shared_ptr<BufferManager> _bufferManager;
        std::shared_ptr<BufferFrame> _bufferFrame;

        SPPage* page() const {
            return reinterpret_cast<SPPage*> (_bufferFrame->getData());
        }
    };
}

#endif	/* SIMPLEDB_DATA_SPITERATOR_HPP */
