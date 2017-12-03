
#include "BufferManager.hpp"

namespace simpledb {

    BufferManager::~BufferManager() {
        // write dirty pages to disk
        for (BufferFrameTableBucket &bucket : _table) {
            std::unique_ptr<boost::unique_lock < boost::mutex>> bucketLock = bucket.lock();

            for (std::shared_ptr<BufferFrame> &frame : bucket) {
                frame->lock(true);
                cleanFrame(frame.get());
                frame->unlock();
            }
        }

        free(_buffer);
    };

    std::shared_ptr<BufferFrame> BufferManager::fixPage(uint64_t segmentId, uint64_t pageId, bool exclusive) {
        PageId page(segmentId, pageId);

        // find the bucket of the page
        BufferFrameTableBucket& bucket = _table.findBucket(page);
        std::shared_ptr<BufferFrame> frame;

        bool deleted = false;
        do {
            // lock bucket
            std::unique_ptr<boost::unique_lock < boost::mutex>> bucketLock = bucket.lock();

            // lookup frame
            frame = bucket.lookupFrame(page);

            // is the frame loaded?
            bool newlyCreated = false;
            if (!frame) {
                newlyCreated = true;

                frame.reset(new BufferFrame(page));
                frame->lock(true); // get exclusive lock until we loaded page from disk
                bucket.insertFrame(frame);

                bucketLock->unlock(); // unlock bucket, so we don't block

                void *data = evictPage();
                frame->setData(data);

                // load data to from disk
                _fileManager.read(page, PAGE_SIZE, data);

                _replacementManager.newFrame(frame.get());

                frame->unlock();
            } else {
                bucketLock->unlock();
            }

            // TODO: if we newly loaded the frame, don't give up lock; if necessary downgrade lock
            // then we won't need to check for deletion (but only if the frame is newly loaded))

            // finally aquire frame lock
            frame->lock(exclusive);

            // aquired lock, now reprioritize
            if (!newlyCreated) {
                _replacementManager.reprioritizeFrame(frame.get());
            }

            // is the frame deleted? -> start from the beginning
            deleted = frame->isDeleted();
            if (deleted) {
                frame->unlock();
            }
        } while (deleted);

        //std::cout << "  fix: " << frame->getData() << std::endl;
        // TODO: really return a normal pointer?
        return frame;
    }

    void BufferManager::unfixPage(std::shared_ptr<BufferFrame> frame, bool isDirty) {
        //std::cout << "unfix: " << frame->getData() << std::endl;

        // is the frame loaded?
        assert(frame.get());
        // why do we call unlock, if the frame doesn't exist / we don't have the lock?!
        // has the caller used unfixPage before fixPage?

        if (isDirty) {
            frame->setDirty();
        }

        frame->unlock();
    }

    void* BufferManager::initBuffer() {
        void *buffer;
        assert(posix_memalign(&buffer, PAGE_SIZE, _size * PAGE_SIZE) == 0);
        assert(buffer != nullptr); // TODO: error handling
        return buffer;
    }

    void* BufferManager::evictPage() {
        BufferFrameNode node = _replacementManager.evictFrame();
        if (node.inUse) {
            // lookup frame in table
            BufferFrameTableBucket& bucket = _table.findBucket(node.page);
            std::unique_ptr<boost::unique_lock < boost::mutex>> bucketLock = bucket.lock();
            std::shared_ptr<BufferFrame> frame = bucket.lookupFrame(node.page);
            assert(frame); // all deletions evict the node fromt the queues first in an atomic operation
            bucketLock->unlock();

            // lock and clean frame
            frame->lock(true);

            cleanFrame(frame.get());
            frame->setDeleted();

            // lock bucket and delete frame
            bucketLock->lock();
            bucket.deleteFrame(node.page);
            bucketLock->unlock();

            frame->unlock();
        }

        return node.data;
    }

    void BufferManager::cleanFrame(BufferFrame* frame) {
        if (frame->isDirty()) {
            // write data to disk
            _fileManager.write(frame->pageId(), PAGE_SIZE, frame->getData());
            frame->setClean();
        }
    }
}
