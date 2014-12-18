#ifndef __usql_select_excutor_h__
#define __usql_select_excutor_h__ value

#include "./sql_statement.h"
#include "./table.h"
#include "./datatype/base.h"
#include "./where_statement.h"
#include "./table_info.h"

using namespace usql;

namespace usql {
    
class SelectExecutor {

private:
    using callback_t = std::function<void(const std::vector<LiteralData> &)>;

    std::vector<LiteralData> callback_values;
    std::vector<std::vector<LiteralData>> verify_values;

    std::vector<std::vector<int>> dests_indexes;
    std::vector<int> table_columns_count; // for index of verify_values

    auto expand_dests(std::vector<ColumnAndTableName> dests) -> decltype(dests);

    void recursive_execute(size_t depth, 
                           std::vector<std::set<rowid_t>> & rows,
                           const std::vector<ColumnAndTableName> dests,
                           const std::unique_ptr<WhereStatement> & where,
                           callback_t callback);

protected:
    std::vector<std::shared_ptr<TableInfo>> tableinfos;
    // std::vector<std::pair<std::string, std::shared_ptr<Table>>> tables;
    // std::map<ColumnAndTableName, std::shared_ptr<IndexBase>> indexes;

public:
    SelectExecutor() = default;
    void addTable(std::shared_ptr<TableInfo> info) {
        tableinfos.push_back(info);
    }
    // void addTable(std::string name, std::shared_ptr<Table> t){
    //     tables.emplace_back(name, t);
    // }
    // void addIndex(ColumnAndTableName name, std::shared_ptr<IndexBase> index) {
    //     indexes[name] = index;
    // }

    void execute(std::vector<ColumnAndTableName> & dests, 
                 const std::unique_ptr<WhereStatement> & where,
                 callback_t callback);

};

}

#endif
