
#ifndef SIMPLEDB_BUFFER_BUFFERFRAMETABLEBUCKET_HPP
#define	SIMPLEDB_BUFFER_BUFFERFRAMETABLEBUCKET_HPP

#include "BufferFrame.hpp"

#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/lock_types.hpp>

#include <cassert>
#include <vector>
#include <memory>

namespace simpledb {

    /**
     * Bucket for buffer frames in hash table.
     * All concurrent operations need an external lock using lock() before lookup, insert or delete.
     * Buffer frames can be accessed without locking the bucket, they have their own locking mechanism.
     */
    class BufferFrameTableBucket {
    public:

        BufferFrameTableBucket() : _mutex(), _bucket() {
        };

        ~BufferFrameTableBucket() {
        };

        BufferFrameTableBucket(const BufferFrameTableBucket& orig) = delete;
        BufferFrameTableBucket& operator=(const BufferFrameTableBucket& orig) = delete;

        /**
         * Locks the bucket.
         * The lock is released when the lock is destructed.
         * @return the lock
         */
        std::unique_ptr<boost::unique_lock<boost::mutex>> lock();

        /**
         * Looks for the frame with the specified pageId in the bucket.
         * The bucket must be locked externally by the caller using lock().
         * @param pageId the page id
         * @return shared_ptr to the frame; or a null shared_ptr, if it can't be found
         */
        std::shared_ptr<BufferFrame> lookupFrame(PageId pageId);

        /**
         * Inserts a frame in the bucket.
         * The bucket must be locked externally by the caller using lock().
         * @param frame the frame to insert
         */
        void insertFrame(std::shared_ptr<BufferFrame> frame);

        /**
         * Deletes a frame from the bucket.
         * The bucket must be locked externally by the caller using lock().
         * @param pageId the page id identifying the frame
         */
        void deleteFrame(PageId pageId);

        std::vector<std::shared_ptr<BufferFrame>>::iterator begin() {
            return _bucket.begin();
        }

        std::vector<std::shared_ptr<BufferFrame>>::iterator end() {
            return _bucket.end();
        }
    private:
        boost::mutex _mutex; // mutex for concurrent access
        std::vector<std::shared_ptr<BufferFrame>> _bucket; // vector of shared_ptr to buffer frames
    };
}

#endif	/* SIMPLEDB_BUFFER_BUFFERFRAMETABLEBUCKET_HPP */

