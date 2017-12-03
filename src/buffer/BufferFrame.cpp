
#include "BufferFrame.hpp"

namespace simpledb {

    void BufferFrame::lock(bool exclusive) {
        if (exclusive) {
            if (_uniqueLock.get() == nullptr) {
                _uniqueLock.reset(new boost::unique_lock<boost::shared_mutex>(_mutex));
            }
        } else {
            if (_sharedLock.get() == nullptr) {
                _sharedLock.reset(new boost::shared_lock<boost::shared_mutex>(_mutex));
            }
        }
    }

    void BufferFrame::unlock() {
        _uniqueLock.reset();
        _sharedLock.reset();
    }
}
