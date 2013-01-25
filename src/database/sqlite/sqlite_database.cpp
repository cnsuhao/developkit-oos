/*
 * This file is part of OpenObjectStore OOS.
 *
 * OpenObjectStore OOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenObjectStore OOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenObjectStore OOS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "database/sqlite/sqlite_database.hpp"
#include "database/sqlite/sqlite_statement.hpp"
#include "database/sqlite/sqlite_result.hpp"
#include "database/sqlite/sqlite_types.hpp"
#include "database/sqlite/sqlite_exception.hpp"

#include "database/session.hpp"
#include "database/transaction.hpp"
#include "database/statement_creator.hpp"
#include "database/row.hpp"

#include "object/object.hpp"
#include "object/object_store.hpp"
#include "object/prototype_node.hpp"

#include <stdexcept>
#include <sqlite3.h>
#include <iostream>

#ifdef WIN32
#include <functional>
#else
#include <tr1/functional>
#endif

using namespace std::tr1::placeholders;

namespace oos {
  
namespace sqlite {
  
sqlite_database::sqlite_database(session *db)
  : database(db, new sqlite_sequencer(this))
  , sqlite_db_(0)
{
}

sqlite_database::~sqlite_database()
{
  close();
}


void sqlite_database::open(const std::string &db)
{
  if (is_open()) {
    return;
  } else {
    int ret = sqlite3_open(db.c_str(), &sqlite_db_);
    if (ret != SQLITE_OK) {
      throw sqlite_exception("couldn't open database: " + db);
    }
    database::open(db);
  }
}

bool sqlite_database::is_open() const
{
  return sqlite_db_ != 0;
}

void sqlite_database::close()
{
  if (!is_open()) {
    return;
  } else {
    database::close();

    sqlite3_close(sqlite_db_);

    sqlite_db_ = 0;
  }
}

void sqlite_database::execute(const char *sql, result_impl *res)
{
  char *errmsg;
  int ret = sqlite3_exec(sqlite_db_, sql, parse_result, res, &errmsg);
  if (ret != SQLITE_OK) {
    std::string error(errmsg);
    sqlite3_free(errmsg);
    throw sqlite_exception(error);
  }
}

result_impl* sqlite_database::create_result()
{
  return new sqlite_static_result;
}

statement* sqlite_database::create_statement()
{
  return new sqlite_statement(*this);
}

void sqlite_database::initialize_table(const prototype_node &node, std::string &create_, std::string &drop_)
{
  // create dummy
  object *o = node.producer->create();
  // create string
  create_statement_creator<sqlite_types> creator;
  create_ = creator.create(o, node.type.c_str(), "");

  // drop string
  drop_statement_creator<sqlite_types> drop;
  drop_ = drop.create(o, node.type.c_str(), "");

  // delete dummy
  delete o;
}

void sqlite_database::prepare_table(const prototype_node &node,                         
                         statement *select, statement *insert,
                         statement *update, statement *remove)
{
  // create dummy
  object *o = node.producer->create();
  
  // prepare select statement
  select_statement_creator<sqlite_types> select_creator;
  // state wasn't found, create sql string
  std::string sql = select_creator.create(o, node.type.c_str(), 0);
  // prepare statement
  select->prepare(sql);
  
  // prepare insert statement
  insert_statement_creator<sqlite_types> insert_creator;
  // state wasn't found, create sql string
  sql = insert_creator.create(o, node.type.c_str(), 0);
  // prepare statement
  insert->prepare(sql);
  
  // prepare insert statement
  update_statement_creator<sqlite_types> update_creator;
  // state wasn't found, create sql string
  sql = update_creator.create(o, node.type.c_str(), "id=?");
  // prepare statement
  update->prepare(sql);
  
  // prepare insert statement
  delete_statement_creator<sqlite_types> delete_creator;
  // state wasn't found, create sql string
  sql = delete_creator.create(o, node.type.c_str(), "id=?");
  // prepare statement
  remove->prepare(sql);
  
  // delete dummy
  delete o;
}

sqlite3* sqlite_database::operator()()
{
  return sqlite_db_;
}

void sqlite_database::on_begin()
{
  execute("BEGIN TRANSACTION;");
}

void sqlite_database::on_commit()
{
  execute("COMMIT TRANSACTION;");
}

void sqlite_database::on_rollback()
{
  execute("ROLLBACK TRANSACTION;");
}

int sqlite_database::parse_result(void* param, int column_count, char** values, char** /*columns*/)
{
  sqlite_static_result *result = static_cast<sqlite_static_result*>(param);

  /********************
   *
   * a new row was retrived
   * add a new row to the result
   * and iterator over all columns
   * adding each column value as
   * string to the row
   *
   ********************/
  std::auto_ptr<row> r(new row);
  for (int i = 0; i < column_count; ++i) {
    r->push_back(std::string(values[i]));
  }
  result->push_back(r.release());
  return 0;
}

}

}

extern "C"
{
  OOS_SQLITE_API oos::database* create_database(oos::session *ses)
  {
    return new oos::sqlite::sqlite_database(ses);
  }

  OOS_SQLITE_API void destroy_database(oos::database *db)
  {
    delete db;
  }
}
