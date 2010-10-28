/*
 * esteid-browser-plugin - a browser plugin for Estonian EID card
 *
 * Copyright (C) 2010  Estonian Informatics Centre
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef JSUTIL_H
#define JSUTIL_H

#define RTERROR_TO_SCRIPT(exp) try { exp; } \
    catch(const std::runtime_error& e) { throw FB::script_error(e.what()); }

#define JS_REGISTER_METHOD(c, a) registerMethod(#a, make_method(this, &c::a));
#define JS_REGISTER_RO_PROPERTY(c, a) registerProperty(#a, \
                                          make_property(this, &c::get_##a));

#endif //JSUTIL_H
