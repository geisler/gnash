// SecurityErrorEvent_as.cpp:  ActionScript "SecurityErrorEvent" class, for Gnash.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "events/SecurityErrorEvent_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value securityerrorevent_toString(const fn_call& fn);
    as_value securityerrorevent_SECURITY_ERROR(const fn_call& fn);
    as_value securityerrorevent_ctor(const fn_call& fn);
    void attachSecurityErrorEventInterface(as_object& o);
    void attachSecurityErrorEventStaticInterface(as_object& o);
    as_object* getSecurityErrorEventInterface();

}

class SecurityErrorEvent_as : public as_object
{

public:

    SecurityErrorEvent_as()
        :
        as_object(getSecurityErrorEventInterface())
    {}
};

// extern (used by Global.cpp)
void
securityerrorevent_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, securityerrorevent_ctor, attachSecurityErrorEventInterface, 
        attachSecurityErrorEventStaticInterface, uri);
}

namespace {

void
attachSecurityErrorEventInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("toString", gl->createFunction(securityerrorevent_toString));
    o.init_member("SECURITY_ERROR", gl->createFunction(securityerrorevent_SECURITY_ERROR));
}

void
attachSecurityErrorEventStaticInterface(as_object& /*o*/)
{
}

as_object*
getSecurityErrorEventInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachSecurityErrorEventInterface(*o);
    }
    return o.get();
}

as_value
securityerrorevent_toString(const fn_call& fn)
{
    boost::intrusive_ptr<SecurityErrorEvent_as> ptr =
        ensureType<SecurityErrorEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
securityerrorevent_SECURITY_ERROR(const fn_call& fn)
{
    boost::intrusive_ptr<SecurityErrorEvent_as> ptr =
        ensureType<SecurityErrorEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
securityerrorevent_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new SecurityErrorEvent_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

