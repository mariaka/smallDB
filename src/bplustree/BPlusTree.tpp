
namespace simpledb {

    template <typename K, typename V, typename C>
    BPlusTree<K, V, C>::BPlusTree(uint64_t segmentId, std::shared_ptr<SegmentManager> segmentManager, std::shared_ptr<BufferManager> bufferManager) : _segmentId(segmentId), _segmentManager(segmentManager), _bufferManager(bufferManager) {
        allocate(MIN_PAGE_NUMBER);
    }

    template <typename K, typename V, typename C>
    void BPlusTree<K, V, C>::insert(K key, V value) {
        { // free space on page? -> normal insert
            std::shared_ptr<BufferFrame> leafFrame = lookupPage(key, true, false);
            LeafNode<K, V, C, LEAF_DEGREE> *leafNode = reinterpret_cast<LeafNode<K, V, C, LEAF_DEGREE>*> (leafFrame->getData());

            if (leafNode->hasFreeSpace()) {
                leafNode->insert(key, value);
                _bufferManager->unfixPage(leafFrame, true);
                return;
            } else {
                _bufferManager->unfixPage(leafFrame, false);
            }
        }

        // no free space on page? -> preventive splitting insert with exclusive access
        PageId rootId(_segmentId, ROOT_PAGE_ID);

        std::shared_ptr<BufferFrame> rootFrame = _bufferManager->fixPage(rootId.segment, rootId.page, true);
        InnerNode<K, V, C, INNER_DEGREE> *rootNode = reinterpret_cast<InnerNode<K, V, C, INNER_DEGREE>*> (rootFrame->getData());

        bool dirtyParent = false;

        // ensure space on root node
        if (!rootNode->hasFreeSpace()) {
            PageId newPages[]{newPage(), newPage()};
            std::shared_ptr<BufferFrame> newFrames[]{
                _bufferManager->fixPage(newPages[0].segment, newPages[0].page, true),
                _bufferManager->fixPage(newPages[1].segment, newPages[1].page, true),
            };
            InnerNode<K, V, C, INNER_DEGREE> *newNodes[]{
                reinterpret_cast<InnerNode<K, V, C, INNER_DEGREE>*> (newFrames[0]->getData()),
                reinterpret_cast<InnerNode<K, V, C, INNER_DEGREE>*> (newFrames[1]->getData())
            };

            rootNode->splitRoot(newPages, newNodes);

            _bufferManager->unfixPage(newFrames[0], true);
            _bufferManager->unfixPage(newFrames[1], true);
            dirtyParent = true;
        }

        std::shared_ptr<BufferFrame> parentFrame = std::move(rootFrame);
        InnerNode<K, V, C, INNER_DEGREE> *parentNode = rootNode;

        PageId nodeId = rootNode->lookup(key);
        std::shared_ptr<BufferFrame> nodeFrame = _bufferManager->fixPage(nodeId.segment, nodeId.page, true);
        Node<K, V, C> *node = reinterpret_cast<Node<K, V, C>*> (nodeFrame->getData());

        while (!node->isLeaf()) {
            InnerNode<K, V, C, INNER_DEGREE> *innerNode = reinterpret_cast<InnerNode<K, V, C, INNER_DEGREE>*> (node);

            // ensure space on current node
            if (!innerNode->hasFreeSpace()) {
                // reserve new page
                PageId newPageId = newPage();
                std::shared_ptr<BufferFrame> newFrame = _bufferManager->fixPage(newPageId.segment, newPageId.page, true);
                InnerNode<K, V, C, INNER_DEGREE> *newNode = reinterpret_cast<InnerNode<K, V, C, INNER_DEGREE>*> (newFrame->getData());

                // split node
                innerNode->split(newPageId, newNode, parentNode);
                dirtyParent = true;

                // unfix splitted nodes
                _bufferManager->unfixPage(nodeFrame, true);
                node = nullptr;
                innerNode = nullptr;
                nodeFrame.reset();
                _bufferManager->unfixPage(newFrame, true);
                newNode = nullptr;
                newFrame.reset();

                // search new node in parent node
                // TODO OPTIMIZATION: don't use normal lookup, there are only two candidates
                nodeId = parentNode->lookup(key);
                nodeFrame = _bufferManager->fixPage(nodeId.segment, nodeId.page, true);
                node = reinterpret_cast<Node<K, V, C>*> (nodeFrame->getData());
                innerNode = reinterpret_cast<InnerNode<K, V, C, INNER_DEGREE>*> (node);
            }

            // unfix parent
            _bufferManager->unfixPage(parentFrame, dirtyParent);
            parentNode = nullptr;
            parentFrame.reset();
            dirtyParent = false;

            // inner node is new parent
            parentFrame = std::move(nodeFrame);
            parentNode = std::move(innerNode);

            // search child page
            nodeId = parentNode->lookup(key);

            // fix child page
            nodeFrame = _bufferManager->fixPage(nodeId.segment, nodeId.page, true);
            node = reinterpret_cast<Node<K, V, C>*> (nodeFrame->getData());
        }

        LeafNode<K, V, C, LEAF_DEGREE> *leafNode = reinterpret_cast<LeafNode<K, V, C, LEAF_DEGREE>*> (node);

        // ensure space on leaf node
        if (!leafNode->hasFreeSpace()) {
            // reserve new page
            PageId newPageId = newPage();
            std::shared_ptr<BufferFrame> newFrame = _bufferManager->fixPage(newPageId.segment, newPageId.page, true);
            LeafNode<K, V, C, LEAF_DEGREE> *newNode = reinterpret_cast<LeafNode<K, V, C, LEAF_DEGREE>*> (newFrame->getData());

            // split node
            leafNode->split(newPageId, newNode, parentNode);
            dirtyParent = true;

            // unfix splitted nodes
            _bufferManager->unfixPage(nodeFrame, true);
            node = nullptr;
            leafNode = nullptr;
            nodeFrame.reset();
            _bufferManager->unfixPage(newFrame, true);
            newNode = nullptr;
            newFrame.reset();

            // search new node in parent node
            // TODO OPTIMIZATION: don't use normal lookup, there are only two candidates
            nodeId = parentNode->lookup(key);
            nodeFrame = _bufferManager->fixPage(nodeId.segment, nodeId.page, true);
            node = reinterpret_cast<Node<K, V, C>*> (nodeFrame->getData());
            leafNode = reinterpret_cast<LeafNode<K, V, C, LEAF_DEGREE>*> (node);
        }

        // unfix parent
        _bufferManager->unfixPage(parentFrame, dirtyParent);
        parentNode = nullptr;
        parentFrame.reset();
        dirtyParent = false;

        // insert entry on leaf node
        leafNode->insert(key, value);

        // unfix leaf page
        _bufferManager->unfixPage(nodeFrame, true);
    }

    template <typename K, typename V, typename C>
    V BPlusTree<K, V, C>::lookup(K key) {
        std::shared_ptr<BufferFrame> nodeFrame = lookupPage(key, false, false);

        // find value
        LeafNode<K, V, C, LEAF_DEGREE> *leafNode = reinterpret_cast<LeafNode<K, V, C, LEAF_DEGREE>*> (nodeFrame->getData());
        V value = leafNode->lookup(key);

        // unfix and return
        _bufferManager->unfixPage(nodeFrame, false);
        return value;
    }

    template <typename K, typename V, typename C>
    typename BPlusTree<K, V, C>::iterator BPlusTree<K, V, C>::lookupRange(K key) {
        std::shared_ptr<BufferFrame> nodeFrame = lookupPage(key, false, false);

        // find index
        LeafNode<K, V, C, LEAF_DEGREE> *leafNode = reinterpret_cast<LeafNode<K, V, C, LEAF_DEGREE>*> (nodeFrame->getData());
        int64_t index = leafNode->lookupIndex(key);

        return BPlusTree<K, V, C>::iterator(index, nodeFrame, _bufferManager);
    }

    template <typename K, typename V, typename C>
    bool BPlusTree<K, V, C>::erase(K key) {
        std::shared_ptr<BufferFrame> nodeFrame = lookupPage(key, true, false);

        // delete entry
        LeafNode<K, V, C, LEAF_DEGREE> *leafNode = reinterpret_cast<LeafNode<K, V, C, LEAF_DEGREE>*> (nodeFrame->getData());
        bool res = leafNode->erase(key);

        // unfix and return
        _bufferManager->unfixPage(nodeFrame, res);
        return res;
    }

    template <typename K, typename V, typename C>
    typename BPlusTree<K, V, C>::size_type BPlusTree<K, V, C>::size() {
        std::shared_ptr<BufferFrame> nodeFrame = lookupPage(K(), false, true);

        BPlusTree<K, V, C>::iterator it(0, nodeFrame, _bufferManager);
        uint64_t i = 0;
        for (; it.isValid(); ++it, ++i);
        return i;
    }

    template <typename K, typename V, typename C>
    void BPlusTree<K, V, C>::visualize(std::ostream& out) {
        out << "digraph myBTree {\n";

        PageId rootId(_segmentId, ROOT_PAGE_ID);
        std::shared_ptr<BufferFrame> nodeFrame = _bufferManager->fixPage(rootId.segment, rootId.page, false);
        InnerNode<K, V, C, INNER_DEGREE> *node = reinterpret_cast<InnerNode<K, V, C, INNER_DEGREE>*> (nodeFrame->getData());

        LeafNode<K, V, C, LEAF_DEGREE> *unusedLeafNode = nullptr;
        node->visualize(rootId, nodeFrame, out, *_bufferManager, unusedLeafNode);

        out << "}\n";
    }

    template <class K, class V, class C>
    std::shared_ptr<BufferFrame> BPlusTree<K, V, C>::lookupPage(K key, bool exclusive, bool leftmost) {
        PageId rootId(_segmentId, ROOT_PAGE_ID);

        std::shared_ptr<BufferFrame> nodeFrame = _bufferManager->fixPage(rootId.segment, rootId.page, false);
        Node<K, V, C> *node = reinterpret_cast<Node<K, V, C>*> (nodeFrame->getData());
        while (!node->isLeaf()) {
            // search child page
            InnerNode<K, V, C, INNER_DEGREE> *innerNode = reinterpret_cast<InnerNode<K, V, C, INNER_DEGREE>*> (node);
            PageId childId;
            if (!leftmost) { // normal key lookup
                childId = innerNode->lookup(key);
            } else { // leftmost child
                childId = innerNode->leftmost();
            }

            // fix child page
            std::shared_ptr<BufferFrame> childFrame = _bufferManager->fixPage(childId.segment, childId.page, false);
            Node<K, V, C> *childNode = reinterpret_cast<Node<K, V, C>*> (nodeFrame->getData());

            // do we need exclusive access on leaf page?
            if (exclusive && childNode->isLeaf()) {
                // unfix and fix with exclusive access, do this BEFORE unfix on parent page! (lock coupling)
                _bufferManager->unfixPage(childFrame, false);
                childNode = nullptr;
                childFrame.reset();
                childFrame = _bufferManager->fixPage(childId.segment, childId.page, exclusive);
            }

            // unfix parent page
            _bufferManager->unfixPage(nodeFrame, false);
            nodeFrame.reset();

            // new node is the child of the old node
            nodeFrame = std::move(childFrame);
            node = reinterpret_cast<Node<K, V, C>*> (nodeFrame->getData());
        }

        return nodeFrame;
    }

    template <class K, class V, class C>
    void BPlusTree<K, V, C>::allocate(uint64_t size) {
        uint64_t oldSize = _segmentManager->retrieve(_segmentId)->size();
        if (oldSize >= size) {
            return;
        }

        _segmentManager->allocate(_segmentId, size, size);
        //uint64_t newSize = _segmentManager->retrieve(_segmentId)->size();

        if (oldSize < MIN_PAGE_NUMBER) { // init
            assert(size >= MIN_PAGE_NUMBER);

            { // meta node
                std::shared_ptr<BufferFrame> metaFrame = _bufferManager->fixPage(_segmentId, META_PAGE_ID, true);
                MetaNode *metaNode = reinterpret_cast<MetaNode*> (metaFrame->getData());
                metaNode->init(3);
                _bufferManager->unfixPage(metaFrame, true);
            }
            { // root node
                std::shared_ptr<BufferFrame> rootFrame = _bufferManager->fixPage(_segmentId, ROOT_PAGE_ID, true);
                InnerNode<K, V, C, INNER_DEGREE> *rootNode = reinterpret_cast<InnerNode<K, V, C, INNER_DEGREE>*> (rootFrame->getData());
                rootNode->init(PageId(_segmentId, FIRST_LEAF_PAGE_ID));
                _bufferManager->unfixPage(rootFrame, true);
            }
            { // first leaf node
                std::shared_ptr<BufferFrame> leafFrame = _bufferManager->fixPage(_segmentId, FIRST_LEAF_PAGE_ID, true);
                LeafNode<K, V, C, LEAF_DEGREE> *leafNode = reinterpret_cast<LeafNode<K, V, C, LEAF_DEGREE>*> (leafFrame->getData());
                leafNode->init();
                _bufferManager->unfixPage(leafFrame, true);
            }
        }
    }

    template <class K, class V, class C>
    PageId BPlusTree<K, V, C>::newPage() {
        std::shared_ptr<BufferFrame> metaFrame = _bufferManager->fixPage(_segmentId, META_PAGE_ID, true);
        MetaNode *metaNode = reinterpret_cast<MetaNode*> (metaFrame->getData());
        uint64_t newPageId = metaNode->nextFreePageId();
        _bufferManager->unfixPage(metaFrame, true);

        allocate(newPageId + 1);
        return PageId(_segmentId, newPageId);
    }
}
