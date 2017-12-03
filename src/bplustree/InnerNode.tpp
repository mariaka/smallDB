
namespace simpledb {

    template <typename K, typename V, typename C, uint64_t DEGREE>
    void InnerNode<K, V, C, DEGREE>::init() {
        this->_isLeaf = false;
        _count = 0;
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    void InnerNode<K, V, C, DEGREE>::init(PageId firstChild) {
        init();
        children()[0] = firstChild;
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    void InnerNode<K, V, C, DEGREE>::insert(K key, PageId pageId) {
        assert(hasFreeSpace());

        typename KeyArray::iterator it = std::lower_bound(keys().begin(), keysEnd(), key, C());
        int k = it - keys().begin();

        std::memmove(&keys()[k + 1], &keys()[k], (size() - k) * sizeof (K));
        std::memmove(&children()[k + 1 + 1], &children()[k + 1], (size() - k) * sizeof (PageId));

        keys()[k] = key;
        children()[k + 1] = pageId;

        ++_count;
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    PageId InnerNode<K, V, C, DEGREE>::lookup(K key) {
        typename KeyArray::iterator it = std::lower_bound(keys().begin(), keysEnd(), key, C());
        return children()[it - keys().begin()];
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    PageId InnerNode<K, V, C, DEGREE>::leftmost() {
        return children()[0];
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    uint64_t InnerNode<K, V, C, DEGREE>::size() {
        return _count;
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    bool InnerNode<K, V, C, DEGREE>::hasFreeSpace() {
        return (size() + 1 < keys().size());
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    void InnerNode<K, V, C, DEGREE>::splitRoot(PageId pageId[2], InnerNode<K, V, C, DEGREE> *node[2]) {
        uint64_t k = size() / 2;

        // setup children
        node[0]->init();
        node[1]->init();

        // keys:     node[0]: 0..k-1, node[1]: k+1..size()-1
        std::memcpy(&node[0]->keys()[0], &this->keys()[0], k * sizeof (K));
        std::memcpy(&node[1]->keys()[0], &this->keys()[k + 1], (size() - k - 1) * sizeof (K));

        // children: node[0]: 0..k,   node[1]: k+1..size()
        std::memcpy(&node[0]->children()[0], &this->children()[0], (k + 1) * sizeof (PageId));
        std::memcpy(&node[1]->children()[0], &this->children()[k + 1], (size() - k) * sizeof (PageId));

        node[0]->_count = k;
        node[1]->_count = size() - k - 1;

        // setup new root
        this->keys()[0] = this->keys()[k];
        this->children()[0] = pageId[0];
        this->children()[1] = pageId[1];
        this->_count = 1;
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    void InnerNode<K, V, C, DEGREE>::split(PageId newPageId, InnerNode<K, V, C, DEGREE> *newNode, InnerNode<K, V, C, DEGREE> *parentNode) {
        uint64_t k = size() / 2;

        // setup new node
        newNode->init();

        // keys:     this: 0..k-1, newNode: k+1..size()-1
        std::memcpy(&newNode->keys()[0], &this->keys()[k + 1], (size() - k - 1) * sizeof (K));

        // children: this: 0..k,   newNode: k+1..size()
        std::memcpy(&newNode->children()[0], &this->children()[k + 1], (size() - k) * sizeof (PageId));

        newNode->_count = size() - k - 1;

        // setup old node
        this->_count = k;

        // insert splitter into root node
        parentNode->insert(this->keys()[k], newPageId);
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    template<uint64_t LEAF_DEGREE>
    void InnerNode<K, V, C, DEGREE>::visualize(PageId thisPageId, std::shared_ptr<BufferFrame> bufferFrame, std::ostream& out, BufferManager& bufferManager, LeafNode<K, V, C, LEAF_DEGREE> *unused) {
        out << "node" << thisPageId.page << " [shape=record, label=\n ";
        out << "\"<count> " << _count << " | <isLeaf> " << std::boolalpha << this->isLeaf();
        for (uint64_t i = 0; i < _count; ++i) {
            out << " | <key" << i << "> " << keys()[i];
        }
        for (uint64_t i = _count; i < keys().size(); ++i) {
            out << " | <key" << i << ">";
        }
        for (uint64_t i = 0; i < _count + 1; ++i) {
            out << " | <ptr" << i << "> *";
        }
        for (uint64_t i = _count + 1; i < children().size(); ++i) {
            out << " | <ptr" << i << ">";
        }
        out << "\"];\n";

        for (uint64_t i = 0; i < _count + 1; ++i) {
            out << "node" << thisPageId.page << ":ptr" << i << " -> node" << children()[i].page << ":count;\n";
        }
        out << "\n";

        bufferManager.unfixPage(bufferFrame, false);

        for (uint64_t i = 0; i < _count + 1; ++i) {
            PageId childId = children()[i];
            std::shared_ptr<BufferFrame> childFrame = bufferManager.fixPage(childId.segment, childId.page, false);
            Node<K, V, C> *childNode = reinterpret_cast<Node<K, V, C>*> (childFrame->getData());

            if (!childNode->isLeaf()) {
                InnerNode<K, V, C, DEGREE> *innerNode = reinterpret_cast<InnerNode<K, V, C, DEGREE>*> (childNode);
                innerNode->visualize(childId, childFrame, out, bufferManager, unused);
            } else {
                LeafNode<K, V, C, LEAF_DEGREE> *leafNode = reinterpret_cast<LeafNode<K, V, C, LEAF_DEGREE>*> (childNode);
                leafNode->visualize(childId, std::move(childFrame), out, bufferManager);
            }
        }
    }
}
