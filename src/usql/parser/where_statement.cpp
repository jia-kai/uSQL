/* 
* @Author: BlahGeek
* @Date:   2014-12-06
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-21
*/

#include <iostream>
#include <algorithm>
#include <utility>
#include "./where_statement.h"
#include "../db/index.h"
#include "../db/table.h"
#include "../db/table_info.h"

using namespace usql;

const rowid_t WhereStatement::INCLUDE_ALL = -2;

std::ostream & WhereStatement::print(std::ostream & stream) const {
    if(type == WhereStatement::WhereStatementType::LEAF) {
        if(a_is_literal) a.print(stream);
        else stream << na.first << "." << na.second;

        stream << " ";
        switch(op) {
            #define DEF(sym, op) \
                case WhereStatement::WhereStatementOperator::sym:\
                    stream << #op ; break;
            DEF(EQ, =)
            DEF(NE, !=)
            DEF(GT, >)
            DEF(LT, <)
            DEF(GE, >=)
            DEF(LE, <=)
            #undef DEF 
            default: stream << "UNKNOWN";
        }
        stream << " ";

        if(b_is_literal) b.print(stream);
        else stream << nb.first << "." << nb.second;
        return stream;
    }
    if(type == WhereStatement::WhereStatementType::PASS)
        return children[0]->print(stream);
    if(type == WhereStatement::WhereStatementType::NOT) {
        stream << "NOT (";
        children[0]->print(stream);
        stream << ")";
        return stream;
    }
    std::string op;
    if(type == WhereStatement::WhereStatementType::AND)
        op = "AND";
    else op = "OR";
    for(size_t i = 0 ; i < children.size() ; i += 1) {
        if(i != 0)
            stream << " " << op << " ";
        auto & child = children[i];
        stream << "("; child->print(stream); stream << ")";
    }
    return stream;
}

bool WhereStatement::verify_leaf() const {
    switch(this->op) {
        #define DEF(sym, op) \
            case WhereStatement::WhereStatementOperator::sym: \
                return a op b;
        DEF(EQ, ==)
        DEF(NE, !=)
        DEF(GT, >)
        DEF(LT, <)
        DEF(GE, >=)
        DEF(LE, <=)
        #undef DEF
        default: return false;
    }
}

void WhereStatement::prepare(const std::vector<std::shared_ptr<TableInfo>> & tableinfos) {
    this->tableinfos = tableinfos;

    if(!a_is_literal) {
        auto tbinfo = std::find_if(tableinfos.begin(), tableinfos.end(),
                                   [&](const std::shared_ptr<TableInfo> & tbinfo) -> bool {
                                      return tbinfo->name == na.first;
                                   });
        usql_assert(tbinfo != tableinfos.end(), "Table %s not found", na.first.c_str());
        na_table_i = tbinfo - tableinfos.begin();

        auto & cols = (*tbinfo)->table->columns;
        auto col = std::find_if(cols.begin(), cols.end(),
                                [&](const column_def_t & col) -> bool {
                                   return col.first == na.second;
                                });
        usql_assert(col != cols.end(), "Col %s (of table %s) not found", 
                    na.second.c_str(), na.first.c_str());
        na_col_i = col - cols.begin();
    }
    if(!b_is_literal) {
        auto tbinfo = std::find_if(tableinfos.begin(), tableinfos.end(),
                                   [&](const std::shared_ptr<TableInfo> & tbinfo) -> bool {
                                      return tbinfo->name == nb.first;
                                   });
        usql_assert(tbinfo != tableinfos.end(), "Table %s not found", nb.first.c_str());
        nb_table_i = tbinfo - tableinfos.begin();

        auto & cols = (*tbinfo)->table->columns;
        auto col = std::find_if(cols.begin(), cols.end(),
                                [&](const column_def_t & col) -> bool {
                                   return col.first == nb.second;
                                });
        usql_assert(col != cols.end(), "Col %s (of table %s) not found", 
                    nb.second.c_str(), nb.first.c_str());
        nb_col_i = col - cols.begin();
    }

    for(auto & child: children)
        child->prepare(tableinfos);
}


bool WhereStatement::verify(const std::vector<std::vector<LiteralData>> & data,
                            bool force_verify) {
    #if 0
    usql_log("In verify: type = %d, op = %d, a_is_l: %d, b_is_l: %d, need_verify: %d",
             int(type), int(op), a_is_literal, b_is_literal, need_verify);
    #endif
    if(type == WhereStatement::WhereStatementType::PASS)
        return children[0]->verify(data, force_verify);
    if(!force_verify && !need_verify)
        return true;
    if(type == WhereStatement::WhereStatementType::LEAF) {
        if(!a_is_literal)
            a = data[na_table_i][na_col_i];
        if(!b_is_literal)
            b = data[nb_table_i][nb_col_i];
        return this->verify_leaf();
    }
#if 0
    if(type == WhereStatement::WhereStatementType::NOT)
        return !(children[0]->verify(data));
#endif
    if(type == WhereStatement::WhereStatementType::AND) {
        for(auto & child: this->children) 
            if(!(child->verify(data)))
                return false;
        return true;
    }
    if(type == WhereStatement::WhereStatementType::OR) {
        for(auto & child: this->children)
            if(child->verify(data, true))
                return true;
        return false;
    }
    usql_assert(false, "not all where statement type is checked");
    return false;
}

WhereStatement::table_rows_map_t WhereStatement::filter(WhereStatement::table_rows_map_t & rows) {
    if(type == WhereStatement::WhereStatementType::PASS)
        return children[0]->filter(rows);
    if(type == WhereStatement::WhereStatementType::LEAF) {
        if(a_is_literal && b_is_literal) {
            if(this->verify_leaf()) return rows;
            else return empty_map(rows);
        }
        if(!a_is_literal && !b_is_literal) {
            need_verify = true;
            return rows;
        }
        if(op == WhereStatement::WhereStatementOperator::NE) {
            need_verify = true;
            return rows;
        }

        this->normalize_leaf();

        auto & index = tableinfos[na_table_i]->indexes[na_col_i];
        if(index == nullptr) {
            need_verify = true;
            return rows;
        }

        WhereStatement::table_rows_map_t copy(rows);
        std::set<rowid_t> orig = rows[na_table_i];
        std::set<rowid_t> & target = copy[na_table_i];

        std::set<rowid_t> ret;
        switch(op) {
            case WhereStatement::WhereStatementOperator::EQ:
                ret = index->find(IndexBase::BoundType::INCLUDE, b,
                                  IndexBase::BoundType::INCLUDE, b);
                break;
            case WhereStatement::WhereStatementOperator::GT:
                ret = index->find(IndexBase::BoundType::EXCLUDE, b,
                                  IndexBase::BoundType::DISABLE, b);
                break;
            case WhereStatement::WhereStatementOperator::LT:
                ret = index->find(IndexBase::BoundType::DISABLE, b,
                                  IndexBase::BoundType::EXCLUDE, b);
                break;
            case WhereStatement::WhereStatementOperator::GE:
                ret = index->find(IndexBase::BoundType::INCLUDE, b,
                                  IndexBase::BoundType::DISABLE, b);
                break;
            case WhereStatement::WhereStatementOperator::LE:
                ret = index->find(IndexBase::BoundType::DISABLE, b,
                                  IndexBase::BoundType::INCLUDE, b);
                break;
            default: break;
        }

        if(target.find(WhereStatement::INCLUDE_ALL) != target.end())
            target = ret;
        else {
            target.clear();
            std::set_intersection(orig.begin(), orig.end(),
                                  ret.begin(), ret.end(),
                                  std::inserter(target, target.begin()));
        }
        return copy;
    }
#if 0
    if(type == WhereStatement::WhereStatementType::NOT) {
        auto ret = children[0]->filter(rows, indexes);
        WhereStatement::table_rows_map_t copy(rows);
        for(auto & it: copy) {
            auto table_name = it.first;
            auto & target = it.second;
            std::set<rowid_t> orig_target = rows[table_name];
            auto & ret_target = ret[table_name];
            if(target.find(WhereStatement::INCLUDE_ALL) != target.end()) {
                if(ret_target.find(WhereStatement::INCLUDE_ALL) != ret_target.end())
                    target.clear();
                else ;
            } else {
                target.clear();
                std::set_difference(orig_target.begin(), orig_target.end(),
                                    ret_target.begin(), ret_target.end(),
                                    std::inserter(target, target.begin()));
            }
        }
        if(children[0]->need_verify)
            this->need_verify = true;
        return copy;
    }
#endif
    if(type == WhereStatement::WhereStatementType::AND) {
        WhereStatement::table_rows_map_t copy(rows);
        for(auto & child: children) {
            auto ret = child->filter(copy);
            for(size_t i = 0 ; i < tableinfos.size() ; i += 1) {
            // for(auto & it: copy) {
                auto table_name = tableinfos[i]->name;
                // auto table_name = it.first;
                auto & target = copy[i];
                // auto & target = it.second;
                std::set<rowid_t> orig_target(target);
                auto & ret_target = ret[i];
                if(target.find(WhereStatement::INCLUDE_ALL) != target.end())
                    target = ret_target;
                else {
                    target.clear();
                    std::set_intersection(orig_target.begin(), orig_target.end(),
                                          ret_target.begin(), ret_target.end(),
                                          std::inserter(target, target.begin()));
                }
            }
        }
        for(auto & child: children)
            if(child->need_verify)
                this->need_verify = true;
        return copy;
    }
    if(type == WhereStatement::WhereStatementType::OR) {
        auto copy = this->empty_map(rows);
        std::set<int> all_changed_tables;
        for(auto & child: children) {
            auto ret = child->filter(rows);
            for(size_t i = 0 ; i < tableinfos.size() ; i += 1) {
            // for(auto & it: copy) {
                auto table_name = tableinfos[i]->name;
                // auto table_name = it.first;
                auto & target = copy[i];
                // auto & target = it.second;
                std::set<rowid_t> orig_target(target);
                auto & ret_target = ret[i];
                // check if multiple table exists
                if(orig_target.size() != ret_target.size())
                    all_changed_tables.insert(i);
                target.clear();
                std::set_union(orig_target.begin(), orig_target.end(),
                               ret_target.begin(), ret_target.end(),
                               std::inserter(target, target.begin()));
            }
        }
        if(all_changed_tables.size() > 1)
            this->need_verify = true;
        for(auto & child: children)
            if(child->need_verify)
                this->need_verify = true;
        return copy;
    }
    usql_assert(false, "not all where statement type is checked");
    return rows;
}

WhereStatement::table_rows_map_t WhereStatement::empty_map(WhereStatement::table_rows_map_t rows) const {
    for(auto & it: rows)
        it.clear();
    return rows;
}

void WhereStatement::normalize() {
    if(type == WhereStatement::WhereStatementType::PASS)
        return children[0]->normalize();
    if(type == WhereStatement::WhereStatementType::LEAF)
        return this->normalize_leaf();
    if(type != WhereStatement::WhereStatementType::NOT) {
        for(auto & child: children)
            child->normalize();
        return;
    }
    children[0]->revert();
    children[0]->normalize();
    type = WhereStatement::WhereStatementType::PASS;
}

void WhereStatement::revert() {
    if(type == WhereStatement::WhereStatementType::LEAF) {
        if(op == WhereStatement::WhereStatementOperator::GT) {
            op = WhereStatement::WhereStatementOperator::LE;
            return;
        }
        if(op == WhereStatement::WhereStatementOperator::LT) {
            op = WhereStatement::WhereStatementOperator::GE;
            return;
        }
        if(op == WhereStatement::WhereStatementOperator::GE) {
            op = WhereStatement::WhereStatementOperator::LT;
            return;
        }
        if(op == WhereStatement::WhereStatementOperator::LE) {
            op = WhereStatement::WhereStatementOperator::GT;
            return;
        }
        if(op == WhereStatement::WhereStatementOperator::EQ) {
            op = WhereStatement::WhereStatementOperator::NE;
            return;
        }
        if(op == WhereStatement::WhereStatementOperator::NE) {
            op = WhereStatement::WhereStatementOperator::EQ;
            return;
        }
    }
    if(type == WhereStatement::WhereStatementType::AND) {
        type = WhereStatement::WhereStatementType::OR;
        for(auto & child: children)
            child->revert();
        return;
    }
    if(type == WhereStatement::WhereStatementType::OR) {
        type = WhereStatement::WhereStatementType::AND;
        for(auto & child: children)
            child->revert();
        return;
    }
    if(type == WhereStatement::WhereStatementType::NOT) {
        type = WhereStatement::WhereStatementType::PASS;
        return;
    }
    if(type == WhereStatement::WhereStatementType::PASS)
        return children[0]->revert();
}

void WhereStatement::normalize_leaf() {
    if(!(a_is_literal && !b_is_literal))
        return;
    std::swap(a, b);
    std::swap(na, nb);
    std::swap(a_is_literal, b_is_literal);

    switch(op) {
        case WhereStatement::WhereStatementOperator::GT:
            op = WhereStatement::WhereStatementOperator::LT; break;
        case WhereStatement::WhereStatementOperator::LT:
            op = WhereStatement::WhereStatementOperator::GT; break;
        case WhereStatement::WhereStatementOperator::GE:
            op = WhereStatement::WhereStatementOperator::LE; break;
        case WhereStatement::WhereStatementOperator::LE:
            op = WhereStatement::WhereStatementOperator::GE; break;
        default: break;
    }
}

void WhereStatement::setDefaultTable(const std::string & tb_name) {
    if(type != WhereStatement::WhereStatementType::LEAF) {
        for(auto & child: children)
            child->setDefaultTable(tb_name);
        return;
    }
    if(!a_is_literal && na.first.length() == 0)
        na.first = tb_name;
    if(!b_is_literal && nb.first.length() == 0)
        nb.first = tb_name;
}
