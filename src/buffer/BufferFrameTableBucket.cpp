
#include "BufferFrameTableBucket.hpp"

namespace simpledb {

    std::unique_ptr<boost::unique_lock<boost::mutex>> BufferFrameTableBucket::lock() {
        return std::unique_ptr<boost::unique_lock < boost::mutex >> (new boost::unique_lock<boost::mutex>(_mutex));
    };

    std::shared_ptr<BufferFrame> BufferFrameTableBucket::lookupFrame(PageId pageId) {
        for (const std::shared_ptr<BufferFrame> &i : _bucket) {
            if (i->pageId() == pageId) {
                return i;
            }
        }

        return std::shared_ptr<BufferFrame>{};
    }

    void BufferFrameTableBucket::insertFrame(std::shared_ptr<BufferFrame> frame) {
        _bucket.push_back(frame);
    };

    void BufferFrameTableBucket::deleteFrame(PageId pageId) {
        for (auto it = _bucket.begin(); it != _bucket.end(); ++it) {
            if ((*it)->pageId() == pageId) {
                _bucket.erase(it);
                break;
            }
        }
    }
}