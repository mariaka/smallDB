
#include "PrintOperator.hpp"

namespace simpledb {

    PrintOperator::PrintOperator(std::unique_ptr<Operator> in, std::ostream& out, std::vector<void (*)(std::ostream&, const Register&) > print) : Operator(), _in(std::move(in)), _out(out), _print(print) {
    }

    void PrintOperator::open() {
        _in->open();
    }

    bool PrintOperator::next() {
        if (_in->next()) {
            std::vector<std::unique_ptr < Register>> tuple = _in->getOutput();
            assert(tuple.size() == _print.size());

            decltype(tuple)::const_iterator it = tuple.cbegin();
            uint64_t i = 0;
            if (it != tuple.cend()) {
                // first column different
                _print[i](_out, *it->get());
                ++it;
                ++i;

                // other columns
                for (; it != tuple.cend(); ++it, ++i) {
                    _out << " | ";
                    _print[i](_out, *it->get());
                }
                _out << '\n';
            }

            return true;
        }
        return false;
    }

    std::vector<std::unique_ptr<Register>> PrintOperator::getOutput() {
        assert(false);
    }

    void PrintOperator::close() {
        _in->close();
    }
}
