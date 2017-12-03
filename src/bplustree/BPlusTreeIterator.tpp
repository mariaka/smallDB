
namespace simpledb {

    template <typename B, bool EXCLUSIVE>
    BPlusTreeIterator<B, EXCLUSIVE>::BPlusTreeIterator(int64_t onPageIt, std::shared_ptr<BufferFrame> bufferFrame, std::shared_ptr<BufferManager> bufferManager) : _bufferManager(bufferManager), _bufferFrame(bufferFrame), _dirty(false), _onPageIt(onPageIt) {
        if (_onPageIt >= leaf()->size()) { // end of page
            operator++();
        }
    }

    template <typename B, bool EXCLUSIVE>
    BPlusTreeIterator<B, EXCLUSIVE>::~BPlusTreeIterator() {
        if (isValid()) {
            _bufferManager->unfixPage(_bufferFrame, _dirty);
        }
    }

    template <typename B, bool EXCLUSIVE>
    void BPlusTreeIterator<B, EXCLUSIVE>::dirty() {
        assert(EXCLUSIVE && isValid());
        _dirty = true;
    }

    template <typename B, bool EXCLUSIVE>
    bool BPlusTreeIterator<B, EXCLUSIVE>::isValid() const {
        if (_onPageIt < 0) {
            return false;
        }
        return true;
    }

    template <typename B, bool EXCLUSIVE>
    std::pair<typename B::key_type, typename B::value_type> BPlusTreeIterator<B, EXCLUSIVE>::operator*() const {
        assert(isValid());
        return std::pair<typename B::key_type, typename B::value_type > (leaf()->key(_onPageIt), leaf()->value(_onPageIt));
    }

    //    template <typename B, bool EXCLUSIVE>
    //    typename BPlusTreeIterator<B, EXCLUSIVE>::value_type& BPlusTreeIterator<B, EXCLUSIVE>::operator*() const {
    //        assert(isValid());
    //        return leaf()->value(_onPageIt);
    //    }

    //    template <typename B, bool EXCLUSIVE>
    //    typename BPlusTreeIterator<B, EXCLUSIVE>::value_type* BPlusTreeIterator<B, EXCLUSIVE>::operator->() const {
    //        assert(isValid());
    //        return &(operator*());
    //    }

    template <typename B, bool EXCLUSIVE>
    BPlusTreeIterator<B, EXCLUSIVE>& BPlusTreeIterator<B, EXCLUSIVE>::operator++() {
        assert(isValid());

        if (!isValid()) {
            return *this;
        }

        // next item on current page?
        if (_onPageIt + 1 < leaf()->size()) {
            ++_onPageIt;
            return *this;
        }

        // search for next non-empty page
        _onPageIt = 0;
        do {
            PageId nextPage = leaf()->nextPage();
            if (!nextPage.isValid()) { // reached end of tree
                _onPageIt = -1;

                // unfix old frame
                if (_dirty) {
                    assert(EXCLUSIVE);
                }
                _bufferManager->unfixPage(_bufferFrame, _dirty);
                _bufferFrame.reset();

                break;
            }

            // fix next frame
            std::shared_ptr<BufferFrame> nextFrame = _bufferManager->fixPage(nextPage.segment, nextPage.page, EXCLUSIVE);

            // unfix old frame
            if (_dirty) {
                assert(EXCLUSIVE);
            }
            _bufferManager->unfixPage(_bufferFrame, _dirty);
            _bufferFrame.reset();

            // set new page and reset dirty flag
            _bufferFrame = std::move(nextFrame);
            _dirty = false;
        } while (leaf()->size() <= 0);

        return *this;
    }
}
