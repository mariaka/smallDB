
namespace simpledb {

    template <typename K, typename V, typename C, uint64_t DEGREE>
    void LeafNode<K, V, C, DEGREE>::insert(K key, V value) {
        assert(hasFreeSpace());

        int k = lookupIndex(key);

        std::memmove(&keys()[k + 1], &keys()[k], (_count - k) * sizeof (K));
        std::memmove(&values()[k + 1], &values()[k], (_count - k) * sizeof (V));

        keys()[k] = key;
        values()[k] = value;

        ++_count;
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    void LeafNode<K, V, C, DEGREE>::init() {
        this->_isLeaf = true;
        _count = 0;
        _next = PageId();
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    V LeafNode<K, V, C, DEGREE>::lookup(K key) {
        typename KeyArray::iterator it = std::lower_bound(keys().begin(), keysEnd(), key, C());

        if (it == keysEnd() || C()(key, *it)) {
            // key not found
            return V();
        }

        return values()[it - keys().begin()];
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    int64_t LeafNode<K, V, C, DEGREE>::lookupIndex(K key) {
        typename KeyArray::iterator it = std::lower_bound(keys().begin(), keysEnd(), key, C());

        return (it - keys().begin());
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    bool LeafNode<K, V, C, DEGREE>::erase(K key) {
        typename KeyArray::iterator it = std::lower_bound(keys().begin(), keysEnd(), key, C());

        if (it == keysEnd() || C()(key, *it)) {
            // key not found
            return false;
        }

        uint64_t k = it - keys().begin();

        std::memmove(&keys()[k], &keys()[k + 1], (_count - k - 1) * sizeof (K));
        std::memmove(&values()[k], &values()[k + 1], (_count - k - 1) * sizeof (V));

        --_count;

        return true;
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    uint64_t LeafNode<K, V, C, DEGREE>::size() {
        return _count;
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    bool LeafNode<K, V, C, DEGREE>::hasFreeSpace() {
        return (size() + 1 < keys().size());
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    PageId LeafNode<K, V, C, DEGREE>::nextPage() {
        return _next;
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    K& LeafNode<K, V, C, DEGREE>::key(uint64_t k) {
        assert(k < size());
        return keys()[k];
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    V& LeafNode<K, V, C, DEGREE>::value(uint64_t k) {
        assert(k < size());
        return values()[k];
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    template <uint64_t INNER_DEGREE>
    void LeafNode<K, V, C, DEGREE>::split(PageId newPageId, LeafNode<K, V, C, DEGREE> *newNode, InnerNode<K, V, C, INNER_DEGREE> *parentNode) {
        uint64_t k = (size() + 1) / 2;

        // setup new node
        newNode->init();

        // keys/values: this: 0..k-1, newNode: k..size()-1
        std::memcpy(&newNode->keys()[0], &this->keys()[k], (size() - k) * sizeof (K));
        std::memcpy(&newNode->values()[0], &this->values()[k], (size() - k) * sizeof (V));

        newNode->_count = size() - k;
        newNode->_next = this->_next;

        // setup old node
        this->_count = k;
        this->_next = newPageId;

        // insert splitter into root node
        parentNode->insert(this->keys()[k - 1], newPageId);
    }

    template <typename K, typename V, typename C, uint64_t DEGREE>
    void LeafNode<K, V, C, DEGREE>::visualize(PageId thisPageId, std::shared_ptr<BufferFrame> bufferFrame, std::ostream& out, BufferManager& bufferManager) {
        out << "node" << thisPageId.page << " [shape=record, label=\n ";
        out << "\"<count> " << _count << " | <isLeaf> " << std::boolalpha << this->isLeaf();
        for (uint64_t i = 0; i < _count; ++i) {
            out << " | <key" << i << "> " << keys()[i];
        }
        for (uint64_t i = _count; i < keys().size(); ++i) {
            out << " | <key" << i << ">";
        }
        for (uint64_t i = 0; i < _count + 1; ++i) {
            out << " | <tid" << i << "> " << values()[i];
        }
        for (uint64_t i = _count + 1; i < values().size(); ++i) {
            out << " | <tid" << i << ">";
        }
        out << " | <next>";
        if (_next.isValid()) {
            out << " *";
        }
        out << "\"];\n";

        if (_next.isValid()) {
            out << "node" << thisPageId.page << ":next" << " -> node";
            out << _next.page;
            out << ":count;\n";
        }
        out << "\n";

        bufferManager.unfixPage(bufferFrame, false);
    }
}
