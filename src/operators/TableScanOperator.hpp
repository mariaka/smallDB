
#ifndef SIMPLEDB_OPERATORS_TABLESCANOPERATOR_HPP
#define SIMPLEDB_OPERATORS_TABLESCANOPERATOR_HPP

#include "Operator.hpp"
#include "Register.hpp"

#include "data.hpp"
#include "schema.hpp"

#include <memory>
#include <vector>

namespace simpledb {

    class TableScanOperator : public Operator {
    public:
        TableScanOperator(Relation& relation, SPSegment& segment);
        virtual ~TableScanOperator() = default;

        TableScanOperator(const TableScanOperator& orig) = delete;
        TableScanOperator& operator=(const TableScanOperator& orig) = delete;

        virtual void open();
        virtual bool next();
        virtual std::vector<std::unique_ptr<Register>> getOutput();
        virtual void close();

    private:
        Relation& _relation;
        SPSegment& _segment;
        std::unique_ptr<SPSegment::iterator> _it;
        std::vector<std::unique_ptr<Register>> _tuple;
    };
}

#endif	/* SIMPLEDB_OPERATORS_TABLESCANOPERATOR_HPP */
