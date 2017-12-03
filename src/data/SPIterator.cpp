
#include "SPIterator.hpp"

namespace simpledb {

    SPIterator::SPIterator(uint64_t segmentId, std::shared_ptr<SegmentManager> segmentManager, std::shared_ptr<BufferManager> bufferManager) : _segmentId(segmentId), _segmentSize(segmentManager->retrieve(segmentId)->size()), _page(0), _slot(-1), _bufferManager(bufferManager), _bufferFrame() {
        if (!isValid()) {
            return;
        }
        _bufferFrame = _bufferManager->fixPage(_segmentId, _page, false);
        operator++();
    }
    
    SPIterator::SPIterator(): _segmentId(0), _segmentSize(0), _page(0), _slot(-1), _bufferManager(), _bufferFrame() {
        assert(!isValid());
    }

    SPIterator::~SPIterator() {
        if (isValid()) {
            _bufferManager->unfixPage(_bufferFrame, false);
        }
    }

    bool SPIterator::isValid() const {
        return (_page < _segmentSize);
    }

    Record SPIterator::operator*() const {
        assert(isValid());

        assert(_slot > -1);
        assert(_slot < static_cast<int64_t> (page()->firstFreeSlot()));
        SPSlot *slot = page()->getSlot(_slot);
        assert(!slot->isFree() && slot->isOnPage());

        return Record(page()->getLength(slot), page()->getItem(slot));
    }

    SPIterator& SPIterator::operator++() {
        if (!isValid()) {
            return *this;
        }

        while (true) {
            // invariants:
            assert(_bufferFrame.get() != nullptr);
            assert(isValid());
            assert(_slot < static_cast<int64_t> (page()->firstFreeSlot()));

            // next slot on page?
            if (_slot + 1 < static_cast<int64_t> (page()->firstFreeSlot())) {
                ++_slot;
                SPSlot *slot = page()->getSlot(_slot);
                if (slot->isFree() || !slot->isOnPage()) {
                    continue; // skip free or redirected slots
                } else {
                    break; // next element found
                }
            } else { // no more slots on current page
                // unfix old page
                _bufferManager->unfixPage(_bufferFrame, false);
                _bufferFrame.reset();

                // go to next page
                ++_page;
                _slot = -1;
                if (!isValid()) {
                    break; // no more elements
                }

                // fix new page
                _bufferFrame = _bufferManager->fixPage(_segmentId, _page, false);
            }
        }

        return *this;
    }
}
