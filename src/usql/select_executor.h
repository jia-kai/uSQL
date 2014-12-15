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
    std::vector<std::pair<std::string, std::shared_ptr<Table>>> tables;
    std::map<ColumnAndTableName, std::shared_ptr<IndexBase>> indexes;

public:
    SelectExecutor() = default;
    void addTable(std::string name, std::shared_ptr<Table> t){
        tables.emplace_back(name, t);
    }
    void addIndex(ColumnAndTableName name, std::shared_ptr<IndexBase> index) {
        indexes[name] = index;
    }

    void execute(std::vector<ColumnAndTableName> dests, 
                 const std::unique_ptr<WhereStatement> & where,
                 callback_t callback);

};

}

#endif
