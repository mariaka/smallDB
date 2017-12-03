
#include "TID.hpp"

namespace simpledb {

    TID::TID(PageId pageId, SlotId slotId) : _pageId(pageId), _slotId(slotId) {
    }
}
