/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * Author: Jan Becker                                                      *
 * Email: jabe@gempa.de                                                    *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 *                                                                         *
 * Other Usage                                                             *
 * Alternatively, this file may be used in accordance with the terms and   *
 * conditions contained in a signed written agreement between you and      *
 * gempa GmbH.                                                             *
 ***************************************************************************/


#ifndef SEISCOMP_SCMASTER_JSON_H__
#define SEISCOMP_SCMASTER_JSON_H__


#include <string>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/memorybuffer.h>
#include <rapidjson/writer.h>


/******************************************************************************
 * To be used along with rapidjson. To make these macros work it is expected
 * to have a variable itr of type
 * rapidjson::Value::ConstMemberIterator. Use JINIT for that.
 * Supported type names are:
 * - Null
 * - False
 * - True
 * - Bool
 * - String
 * - Object
 * - Array
 * - Number
 * - Int
 * - Uint
 * - Int64
 * - Uint64
 * - Double
 * To retrieve the value, use Get[type]() on the Value object.
 * ----------------------------------------------------------------------------
 * Usage example:
 *
 * void parseSomething() {
 *     rapidjson::Document doc;
 *     JINIT; // Create temporary iterator
 *     doc.Parse("...");
 *     // Check for member "test" with type Object. If thats fails, return
 *     if ( JFAIL(doc, "test", Object) )
 *         return;
 *     // Store the last found member value
 *     const rapidjson::Value &test = JVAL;
 *     if ( JOK(test, "attrib1", String) )
 *         cout << JNAME.GetString() << " = " << JVAL.GetString() << endl;
 *     ...
 * }
 ******************************************************************************/
#define JINIT rapidjson::Value::ConstMemberIterator jitr
#define JHAS(node, member) (jitr = node.FindMember(member)) != node.MemberEnd()
#define JMISS(node, member) (jitr = node.FindMember(member)) == node.MemberEnd()
#define JFAIL(node, member, type) ((jitr = node.FindMember(member)) == node.MemberEnd() || !jitr->value.Is##type())
#define JOK(node, member, type) ((jitr = node.FindMember(member)) != node.MemberEnd() && jitr->value.Is##type())
#define JNAME jitr->name
#define JVAL jitr->value


/**
 * @brief The StdStringBuffer struct implements a rapidjson output buffer
 *
 * It writes to a passed std::string without doing copies later.
 */
struct StdStringBuffer {
	typedef std::string::value_type Ch; // byte

	explicit StdStringBuffer(std::string &out) : out_(out) {}

	void Put(Ch c) { out_.push_back(c); }
	void Flush() {}

	std::string &out_;
};


#endif
