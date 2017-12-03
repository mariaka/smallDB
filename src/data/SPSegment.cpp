
#include "SPSegment.hpp"

namespace simpledb {

    SPSegment::SPSegment(uint64_t segmentId, std::shared_ptr<SegmentManager> segmentManager, std::shared_ptr<BufferManager> bufferManager) : _segmentId(segmentId), _segmentManager(segmentManager), _bufferManager(bufferManager) {

    }

    TID SPSegment::insert(const Record& record) {
        uint64_t pageId;
        std::shared_ptr<BufferFrame> bufferFrame;
        SPPage *page;

        std::tie(pageId, bufferFrame, page) = searchFreeSpace(record.length());

        SlotId slotId = page->insert(record);

        _bufferManager->unfixPage(bufferFrame, true);

        //std::cout << "insert: " << pageId << ", " << slotId << " (len = " << record.length() << ")" << std::endl;
        return (TID(PageId(_segmentId, pageId), slotId));
    }

    bool SPSegment::remove(TID tid) {
        assert(tid.pageId().segment == _segmentId);

        std::shared_ptr<BufferFrame> bufferFrame = _bufferManager->fixPage(tid.pageId().segment, tid.pageId().page, true);
        SPPage *page = reinterpret_cast<SPPage *> (bufferFrame->getData());
        SPSlot *slot = page->getSlot(tid.slotId());

        // record doesn't exist
        if (slot->isFree()) {
            _bufferManager->unfixPage(bufferFrame, false);
            return false;
        }

        assert(!slot->isRedirected()); // use a redirected TID for remove?

        page->remove(tid.slotId());

        // record is a redirect
        if (!slot->isOnPage()) {
            TID redirectedTid = page->getRedirectedTID(slot);
            _bufferManager->unfixPage(bufferFrame, true);


            bufferFrame = _bufferManager->fixPage(redirectedTid.pageId().segment, redirectedTid.pageId().page, false);
            page = reinterpret_cast<SPPage *> (bufferFrame->getData());
            slot = page->getSlot(redirectedTid.slotId());

            assert(!slot->isFree() && slot->isOnPage() && slot->isRedirected());

            page->remove(redirectedTid.slotId());
        }

        _bufferManager->unfixPage(bufferFrame, true);

        return true;
    }

    Record SPSegment::lookup(TID tid) {
        assert(tid.pageId().segment == _segmentId);

        std::shared_ptr<BufferFrame> bufferFrame = _bufferManager->fixPage(tid.pageId().segment, tid.pageId().page, false);
        SPPage *page = reinterpret_cast<SPPage *> (bufferFrame->getData());
        SPSlot *slot = page->getSlot(tid.slotId());

        // record doesn't exist
        if (slot->isFree()) {
            _bufferManager->unfixPage(bufferFrame, false);
            return Record();
        }

        // record is on page
        if (slot->isOnPage()) {
            Record record(page->getLength(slot), page->getItem(slot));
            _bufferManager->unfixPage(bufferFrame, false);
            return record;
        }

        // record is a redirect
        TID redirectedTid = page->getRedirectedTID(slot);
        _bufferManager->unfixPage(bufferFrame, false);


        bufferFrame = _bufferManager->fixPage(redirectedTid.pageId().segment, redirectedTid.pageId().page, false);
        page = reinterpret_cast<SPPage *> (bufferFrame->getData());
        slot = page->getSlot(redirectedTid.slotId());

        assert(!slot->isFree() && slot->isOnPage() && slot->isRedirected());

        Record record(page->getLength(slot), page->getItem(slot));
        _bufferManager->unfixPage(bufferFrame, false);
        return record;
    }

    bool SPSegment::update(TID tid, const Record& record) {
        assert(tid.pageId().segment == _segmentId);

        std::shared_ptr<BufferFrame> bufferFrame = _bufferManager->fixPage(tid.pageId().segment, tid.pageId().page, true);
        SPPage *page = reinterpret_cast<SPPage *> (bufferFrame->getData());
        SPSlot *slot = page->getSlot(tid.slotId());

        // record doesn't exist
        if (slot->isFree()) {
            _bufferManager->unfixPage(bufferFrame, false);
            return false;
        }

        assert(!slot->isRedirected()); // use a redirected TID for update?

        // record is on page
        if (slot->isOnPage()) {
            // new record not longer than old one -> update in place
            if (record.length() <= page->getLength(slot)) {
                page->updateInPlace(tid.slotId(), record);
                _bufferManager->unfixPage(bufferFrame, true);
                return true;
            }

            // new record has enough space on page -> update on page
            if (page->hasUpdateSpace(record.length())) {
                page->updateOnPage(tid.slotId(), record);
                _bufferManager->unfixPage(bufferFrame, true);
                return true;
            }

            // not enough space on page -> redirect to new page
            {
                _bufferManager->unfixPage(bufferFrame, false);
                slot = nullptr;

                // find a redirected entry
                uint64_t redirectedPageId;
                std::tie(redirectedPageId, bufferFrame, page) = searchFreeSpace(record.length() + sizeof (TID));

                SlotId redirectedSlotId = page->insert(record, true, tid);
                _bufferManager->unfixPage(bufferFrame, true);

                // set redirect on original page
                bufferFrame = _bufferManager->fixPage(tid.pageId().segment, tid.pageId().page, true);
                page = reinterpret_cast<SPPage *> (bufferFrame->getData());
                slot = page->getSlot(tid.slotId());
                TID redirectedTid = TID(PageId(_segmentId, redirectedPageId), redirectedSlotId);

                page->updateInPlace(tid.slotId(), Record(sizeof (TID), reinterpret_cast<char*> (&redirectedTid)));
                slot->setOnPage(false);

                _bufferManager->unfixPage(bufferFrame, true);
                return true;
            }
        }

        // record is a redirect
        TID redirectedTid = page->getRedirectedTID(slot);
        _bufferManager->unfixPage(bufferFrame, false);

        bufferFrame = _bufferManager->fixPage(redirectedTid.pageId().segment, redirectedTid.pageId().page, true);
        page = reinterpret_cast<SPPage *> (bufferFrame->getData());
        slot = page->getSlot(redirectedTid.slotId());

        assert(!slot->isFree() && slot->isOnPage() && slot->isRedirected());

        // new record not longer than old one -> update in place
        if (record.length() <= page->getLength(slot)) {
            page->updateInPlace(redirectedTid.slotId(), record);
            _bufferManager->unfixPage(bufferFrame, true);
            return true;
        }

        // new record has enough space on page -> update on page
        if (page->hasUpdateSpace(record.length() + sizeof (TID))) {
            page->updateOnPage(redirectedTid.slotId(), record);
            _bufferManager->unfixPage(bufferFrame, true);
            return true;
        }

        // not enough space on page -> delete old redirect, redirect to new page
        {
            // delete old redirected entry
            page->remove(redirectedTid.slotId());
            _bufferManager->unfixPage(bufferFrame, true);
            slot = nullptr;

            // TODO: try to insert on original page first

            // find new redirected entry
            uint64_t newRedirectedPageId;
            std::tie(newRedirectedPageId, bufferFrame, page) = searchFreeSpace(record.length() + sizeof (TID));

            SlotId newRedirectedSlotId = page->insert(record, true, tid);
            _bufferManager->unfixPage(bufferFrame, true);

            // set redirect on original page
            bufferFrame = _bufferManager->fixPage(tid.pageId().segment, tid.pageId().page, true);
            page = reinterpret_cast<SPPage *> (bufferFrame->getData());
            slot = page->getSlot(tid.slotId());
            TID newRedirectedTid = TID(PageId(_segmentId, newRedirectedPageId), newRedirectedSlotId);

            slot->setOnPage(true);
            page->updateInPlace(tid.slotId(), Record(sizeof (TID), reinterpret_cast<char*> (&newRedirectedTid)));
            slot->setOnPage(false);

            _bufferManager->unfixPage(bufferFrame, true);
            return true;
        }
    }

    std::unique_ptr<SPSegment::iterator> SPSegment::range() {
        return std::unique_ptr<SPSegment::iterator>(new iterator(_segmentId, _segmentManager, _bufferManager));
    }

    std::tuple<uint64_t, std::shared_ptr<BufferFrame>, SPPage*> SPSegment::searchFreeSpace(uint64_t size) {
        uint64_t segmentSize = _segmentManager->retrieve(_segmentId)->size();

        uint64_t pageId;
        std::shared_ptr<BufferFrame> bufferFrame;
        SPPage *page;

        for (pageId = 0; pageId < segmentSize; ++pageId) {
            bufferFrame = _bufferManager->fixPage(_segmentId, pageId, true);
            // TODO: don't use exclusive access to search for free space
            page = reinterpret_cast<SPPage *> (bufferFrame->getData());

            if (!page->hasFreeSpace(size)) { // no free space on page
                _bufferManager->unfixPage(bufferFrame, false);
                continue;
            } else { // page with free space found
                break;
            }
        }

        // no free space found => allocate new page
        if (pageId == segmentSize) {
            allocate(segmentSize + 1);

            bufferFrame = _bufferManager->fixPage(_segmentId, pageId, true);
            page = reinterpret_cast<SPPage *> (bufferFrame->getData());
        }

        return std::make_tuple(pageId, bufferFrame, page);
    }

    void SPSegment::allocate(uint64_t size) {
        uint64_t oldSize = _segmentManager->retrieve(_segmentId)->size();

        _segmentManager->allocate(_segmentId, size, size);
        uint64_t newSize = _segmentManager->retrieve(_segmentId)->size();

        for (uint64_t i = oldSize; i < newSize; ++i) {
            std::shared_ptr<BufferFrame> bufferFrame = _bufferManager->fixPage(_segmentId, i, true);
            SPPage *page = reinterpret_cast<SPPage *> (bufferFrame->getData());
            page->init(_bufferManager->pageSize());
            _bufferManager->unfixPage(bufferFrame, true);
        }
    }
}
