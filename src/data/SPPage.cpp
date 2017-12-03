
#include "SPPage.hpp"

namespace simpledb {

    constexpr uint64_t SPPage::MINIMAL_RECORD_LENGTH;

    void SPPage::init(uint64_t pageSize) {
        _slotCount = 0;
        _firstFreeSlot = 0;
        _dataStart = pageSize;
        _freeSpace = pageSize - sizeof (SPPage);
    }

    bool SPPage::hasFreeSpace(uint64_t size) {
        if (_freeSpace < std::max(size, MINIMAL_RECORD_LENGTH) + sizeof (SPSlot)) {
            return false;
        }
        if (_dataStart - sizeof (SPPage) - _firstFreeSlot * sizeof (SPSlot) < std::max(size, MINIMAL_RECORD_LENGTH) + sizeof (SPSlot)) {
            return false;
        }
        return true;
    }

    bool SPPage::hasUpdateSpace(uint64_t size) {
        if (_dataStart - sizeof (SPPage) - _firstFreeSlot * sizeof (SPSlot) < std::max(size, MINIMAL_RECORD_LENGTH)) {
            return false;
        }
        return true;
    }

    std::tuple<uint64_t, SPSlot*> SPPage::nextSlot() {
        uint64_t slotId = _firstFreeSlot;

        ++_firstFreeSlot;
        ++_slotCount;
        _freeSpace -= sizeof (SPSlot);

        SPSlot *slot = getSlot(slotId);

        return std::make_tuple(slotId, slot);
    }

    SlotId SPPage::insert(const Record& record, bool redirected, TID originalTid) {
        assert(hasFreeSpace(std::max(record.length(), MINIMAL_RECORD_LENGTH) + (redirected ? sizeof (TID) : 0)));

        SlotId slotId;
        SPSlot *slot;
        std::tie(slotId, slot) = nextSlot();

        uint64_t length = record.length();
        uint64_t offset = _dataStart - std::max(length, MINIMAL_RECORD_LENGTH);
        slot->setLength(length);
        slot->setOffset(offset);
        slot->setOnPage(true);
        slot->setRedirected(redirected);

        if (record.getData()) {
            memcpy(reinterpret_cast<char*> (this) + offset, record.getData(), length);
        }
        _dataStart -= std::max(length, MINIMAL_RECORD_LENGTH);
        _freeSpace -= std::max(length, MINIMAL_RECORD_LENGTH);

        if (redirected) {
            memcpy(reinterpret_cast<char*> (this) + offset - sizeof (TID), &originalTid, sizeof (TID));
            _dataStart -= sizeof (TID);
            _freeSpace -= sizeof (TID);
        }

        return slotId;
    }

    void SPPage::remove(SlotId slotId) {
        SPSlot* slot = getSlot(slotId);

        --_slotCount;
        _freeSpace += sizeof (SPSlot);
        _freeSpace += std::max(getLength(slot), MINIMAL_RECORD_LENGTH);
        if (slot->isRedirected()) {
            _freeSpace += sizeof (TID);
        }

        slot->setOffset(0);
        slot->setLength(0);
    }

    void SPPage::updateInPlace(SlotId slotId, const Record& record) {
        SPSlot *slot = getSlot(slotId);
        assert(slot->isOnPage());

        uint64_t oldLength = getLength(slot);
        uint64_t newLength = record.length();
        assert(std::max(newLength, MINIMAL_RECORD_LENGTH) <= oldLength);
        uint64_t oldOffset = slot->offset();
        uint64_t newOffset = oldOffset + (oldLength - std::max(newLength, MINIMAL_RECORD_LENGTH));

        slot->setOffset(newOffset);
        slot->setLength(newLength);

        if (record.getData()) {
            memcpy(reinterpret_cast<char*> (this) + newOffset, record.getData(), newLength);
        }
        _freeSpace += oldLength - std::max(newLength, MINIMAL_RECORD_LENGTH);

        if (slot->isRedirected()) {
            memmove(reinterpret_cast<char*> (this) + newOffset - sizeof (TID), reinterpret_cast<char*> (this) + oldOffset - sizeof (TID), sizeof (TID));
        }
    }

    void SPPage::updateOnPage(SlotId slotId, const Record& record) {
        SPSlot *slot = getSlot(slotId);
        assert(slot->isOnPage());

        _freeSpace += std::max(getLength(slot), MINIMAL_RECORD_LENGTH);

        uint64_t oldOffset = slot->offset();
        uint64_t length = record.length();
        uint64_t offset = _dataStart - std::max(length, MINIMAL_RECORD_LENGTH);
        slot->setLength(length);
        slot->setOffset(offset);

        if (record.getData()) {
            memcpy(reinterpret_cast<char*> (this) + offset, record.getData(), length);
        }
        _dataStart -= std::max(length, MINIMAL_RECORD_LENGTH);
        _freeSpace -= std::max(length, MINIMAL_RECORD_LENGTH);

        if (slot->isRedirected()) {
            memmove(reinterpret_cast<char*> (this) + offset - sizeof (TID), reinterpret_cast<char*> (this) + oldOffset - sizeof (TID), sizeof (TID));
            _dataStart -= sizeof (TID);
        }
    }
}
