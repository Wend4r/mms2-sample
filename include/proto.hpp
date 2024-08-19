/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * Metamod:Source {project}
 * Written by {name of author} ({fullname}).
 * ======================================================

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _INCLUDE_METAMOD_SOURCE_PROTO_HPP_
#	define _INCLUDE_METAMOD_SOURCE_PROTO_HPP_

#	include <string>

#	define PROTO_CONCAT_PROTO_MEMBER_BASE(message, member, padding_str) #member, padding_str

#	define PROTO_CONCAT_PROTO_MEMBER(message, padding_str, member) PROTO_CONCAT_PROTO_MEMBER_BASE(message, member, padding_str), message.member()
#	define PROTO_CONCAT_PROTO_MEMBER_TO_C(message, padding_str, member) PROTO_CONCAT_PROTO_MEMBER(message, padding_str, member).c_str()
#	define PROTO_CONCAT_PROTO_MEMBER_TO_C_STRING(message, padding_str, member) PROTO_CONCAT_PROTO_MEMBER_TO_C(message, padding_str, member).c_str()
#	define PROTO_CONCAT_PROTO_MEMBER_TO_C_BOOLEAN(message, padding_str, member) PROTO_CONCAT_PROTO_MEMBER(message, padding_str, member) ? "true" : "false"

#	define PROTO_MEMEBER_TO_STRING(message, member) std::to_string(config.member())

#endif //_INCLUDE_METAMOD_SOURCE_PROTO_HPP_
