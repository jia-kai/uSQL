#ifndef __usql_select_excutor_h__
#define __usql_select_excutor_h__ value

#include "./sql_statement.h"
#include "./table.h"
#include "./datatype/base.h"
#include "./where_statement.h"

using namespace usql;

namespace usql {
    
class SelectExecutor {

private:
    using callback_t = std::function<void(const std::vector<LiteralData> &)>;

    std::vector<LiteralData> callback_values;
    std::vector<LiteralData> verify_values;

    std::vector<std::vector<int>> dests_indexes;

    void recursive_execute(size_t depth, 
                           std::map<std::string, std::set<rowid_t>> & rows,
                           const std::vector<ColumnAndTableName> dests,
                           const std::unique_ptr<WhereStatement> & where,
                           callback_t callback);

protected:
    std::vector<std::pair<std::string, std::unique_ptr<Table>>> tables;
    std::map<ColumnAndTableName, std::unique_ptr<IndexBase>> indexes;

public:
    SelectExecutor() = default;
    void addTable(std::string name, std::unique_ptr<Table> && t){
        tables.emplace_back(name, std::move(t));
    }
    void addIndex(ColumnAndTableName name, std::unique_ptr<IndexBase> && index) {
        indexes[name] = std::move(index);
    }

    void execute(std::vector<ColumnAndTableName> dests, 
                 const std::unique_ptr<WhereStatement> & where,
                 callback_t callback);

};

}

#endif
