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

#ifndef SERIALIZABLE_HPP
#define SERIALIZABLE_HPP

#ifdef _MSC_VER
  #ifdef oos_EXPORTS
    #define OOS_API __declspec(dllexport)
    #define EXPIMP_TEMPLATE
  #else
    #define OOS_API __declspec(dllimport)
    #define EXPIMP_TEMPLATE extern
  #endif
  #pragma warning(disable: 4251)
#else
  #define OOS_API
#endif

namespace oos {

class object_writer;
class object_reader;

/**
 * @class serializable
 * @brief Base class for all serializable objects
 *
 * This class provides an interface to serialize
 * and deserialize an serializable. It is used with
 * an instance of a concrete object_atomizer class.
 */
class OOS_API serializable
{
public:
  virtual ~serializable() {}

  /**
   * @fn virtual void deserialize(object_reader &deserializer)
   * Deserializes an serializable from the given
   * object_atomizer serializable.
   *
   * @param deserializer The object_atomizer to deserialize from.
   */
  virtual void deserialize(object_reader &) = 0;

  /**
   * @fn virtual void serialize(object_writer &serializer) const
   * Serializes an serializable to the given
   * object_atomizer serializable.
   *
   * @param serializer The object_atomizer to serialize to.
   */
  virtual void serialize(object_writer &) const = 0;
};

}
#endif /* SERIALIZABLE */