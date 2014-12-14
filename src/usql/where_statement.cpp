/* 
* @Author: BlahGeek
* @Date:   2014-12-06
* @Last Modified by:   BlahGeek
* @Last Modified time: 2014-12-14
*/

#include <iostream>
#include <algorithm>
#include <utility>
#include "./where_statement.h"
#include "./index.h"
#include "./table.h"

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

void WhereStatement::prepare_verify(const std::vector<ColumnAndTableName> & names) {
    if(type != WhereStatement::WhereStatementType::LEAF) {
        for(auto & child: children)
            child->prepare_verify(names);
        return;
    }
    if(!a_is_literal) {
        auto it = std::find(names.begin(), names.end(), na);
        usql_assert(it != names.end(), "na: %s.%s not found", 
                    na.first.c_str(), na.second.c_str());
        verify_index_a = it - names.begin();
    }
    if(!b_is_literal) {
        auto it = std::find(names.begin(), names.end(), nb);
        usql_assert(it != names.end(), "nb: %s.%s not found", 
                    nb.first.c_str(), nb.second.c_str());
        verify_index_b = it - names.begin();
    }
}


bool WhereStatement::verify(const std::vector<LiteralData> & data,
                            bool force_verify) {
    if(type == WhereStatement::WhereStatementType::PASS)
        return children[0]->verify(data, force_verify);
    if(!force_verify && !need_verify)
        return true;
    if(type == WhereStatement::WhereStatementType::LEAF) {
        if(a_is_literal)
            a = data[verify_index_a];
        if(b_is_literal)
            b = data[verify_index_b];
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
            if(child->verify(data))
                return true;
        return false;
    }
    usql_assert(false, "not all where statement type is checked");
    return false;
}

WhereStatement::table_rows_map_t WhereStatement::filter(WhereStatement::table_rows_map_t & rows,
                                                        WhereStatement::index_map_t & indexes) {
    if(type == WhereStatement::WhereStatementType::PASS)
        return children[0]->filter(rows, indexes);
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
        auto index_it = indexes.find(na);
        if(index_it == indexes.end()){
            need_verify = true;
            return rows;
        } // not indexed
        auto & index = index_it->second;

        WhereStatement::table_rows_map_t copy(rows);
        std::set<rowid_t> orig = rows[na.first];
        std::set<rowid_t> & target = copy[na.first];

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
            auto ret = child->filter(copy, indexes);
            for(auto & it: copy) {
                auto table_name = it.first;
                auto & target = it.second;
                std::set<rowid_t> orig_target(target);
                auto & ret_target = ret[table_name];
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
        std::set<std::string> all_changed_tables;
        for(auto & child: children) {
            auto ret = child->filter(rows, indexes);
            for(auto & it: copy) {
                auto table_name = it.first;
                auto & target = it.second;
                std::set<rowid_t> orig_target(target);
                auto & ret_target = ret[table_name];
                // check if multiple table exists
                if(orig_target.size() != ret_target.size())
                    all_changed_tables.insert(table_name);
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
        it.second.clear();
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
        if(op == WhereStatement::WhereStatementOperator::GT)
            op = WhereStatement::WhereStatementOperator::LE;
        if(op == WhereStatement::WhereStatementOperator::LT)
            op = WhereStatement::WhereStatementOperator::GE;
        if(op == WhereStatement::WhereStatementOperator::GE)
            op = WhereStatement::WhereStatementOperator::LT;
        if(op == WhereStatement::WhereStatementOperator::LE)
            op = WhereStatement::WhereStatementOperator::GT;
        return;
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
    if(op == WhereStatement::WhereStatementOperator::GT)
        op = WhereStatement::WhereStatementOperator::LT;
    if(op == WhereStatement::WhereStatementOperator::LT)
        op = WhereStatement::WhereStatementOperator::GT;
    if(op == WhereStatement::WhereStatementOperator::GE)
        op = WhereStatement::WhereStatementOperator::LE;
    if(op == WhereStatement::WhereStatementOperator::LE)
        op = WhereStatement::WhereStatementOperator::GE;
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
