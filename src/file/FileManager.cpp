
#include "FileManager.hpp"

namespace simpledb {

    FileManager::~FileManager() {
        for (auto i : _fileHandles) {
            ::close(i.second);
            // TODO: error handling
        }
    }

    void FileManager::read(PageId pageId, const uint64_t PAGE_SIZE, void *data) {
        if (!isOpen(pageId.segment)) {
            open(pageId.segment);
        }

        int res = ::pread(_fileHandles.at(pageId.segment), data, PAGE_SIZE, pageId.page * PAGE_SIZE);
        assert(res > -1);
        // TODO: error handling
    }

    void FileManager::write(PageId pageId, const uint64_t PAGE_SIZE, void *data) {
        if (!isOpen(pageId.segment)) {
            open(pageId.segment);
        }

        int res = ::pwrite(_fileHandles.at(pageId.segment), data, PAGE_SIZE, pageId.page * PAGE_SIZE);
        assert(res > -1);
        // TODO: error handling
    }

    void FileManager::create(uint64_t segmentId) {
        boost::lock_guard<boost::mutex> lock(_mutex);

        assert(!isOpen(segmentId));
        assert(true /* TODO: assert file doesn't exist */);

        int fd = ::open(getFileName(segmentId).c_str(), O_RDWR | O_CREAT | O_SYNC, S_IRUSR | S_IWUSR);
        assert(fd > -1);
        // TODO: error handling

        _fileHandles[segmentId] = fd;
    }

    void FileManager::remove(uint64_t segmentId) {
        boost::lock_guard<boost::mutex> lock(_mutex);

        closeUnsyncronized(segmentId);

        int res = ::unlink(getFileName(segmentId).c_str());
        assert(res > -1);
        // TODO: error handling
    }

    void FileManager::truncate(uint64_t segmentId, const uint64_t PAGE_SIZE, uint64_t size) {
        boost::lock_guard<boost::mutex> lock(_mutex);

        if (isOpen(segmentId)) {
            int res = ::ftruncate(_fileHandles.at(segmentId), size * PAGE_SIZE);
            assert(res > -1);
            // TODO: error handling
        }
        else {
            int res = ::truncate(getFileName(segmentId).c_str(), size * PAGE_SIZE);
            assert(res > -1);
            // TODO: error handling
        }
    }

    void FileManager::open(uint64_t segmentId) {
        boost::lock_guard<boost::mutex> lock(_mutex);

        if (isOpen(segmentId)) {
            return;
        }

        int fd = ::open(getFileName(segmentId).c_str(), O_RDWR | O_SYNC);
        assert(fd > -1);
        // TODO: error handling

        _fileHandles[segmentId] = fd;
    }

    void FileManager::close(uint64_t segmentId) {
        boost::lock_guard<boost::mutex> lock(_mutex);

        closeUnsyncronized(segmentId);
    }

    void FileManager::closeUnsyncronized(uint64_t segmentId) {
        if (!isOpen(segmentId)) {
            return;
        }

        int fd = _fileHandles[segmentId];
        _fileHandles.erase(fd);

        int res = ::close(fd);
        assert(res > -1);
        // TODO: error handling
    }
}