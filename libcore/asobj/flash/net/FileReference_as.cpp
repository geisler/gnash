// filereference_as.cpp:  ActionScript "FileReference" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "FileReference_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics

#include <sstream>

namespace gnash {

static as_value filereference_addListener(const fn_call& fn);
static as_value filereference_browse(const fn_call& fn);
static as_value filereference_cancel(const fn_call& fn);
static as_value filereference_download(const fn_call& fn);
static as_value filereference_removeListener(const fn_call& fn);
static as_value filereference_upload(const fn_call& fn);
static as_value filereference_creationDate_getset(const fn_call& fn);
static as_value filereference_creator_getset(const fn_call& fn);
static as_value filereference_modificationDate_getset(const fn_call& fn);
static as_value filereference_name_getset(const fn_call& fn);
static as_value filereference_size_getset(const fn_call& fn);
static as_value filereference_type_getset(const fn_call& fn);
as_value filereference_ctor(const fn_call& fn);

void attachFileReferenceStaticInterface(as_object& /*o*/)
{

}

static void
attachFileReferenceInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("addListener", gl->createFunction(filereference_addListener));
    o.init_member("browse", gl->createFunction(filereference_browse));
    o.init_member("cancel", gl->createFunction(filereference_cancel));
    o.init_member("download", gl->createFunction(filereference_download));
    o.init_member("removeListener", gl->createFunction(filereference_removeListener));
    o.init_member("upload", gl->createFunction(filereference_upload));
    o.init_property("creationDate", filereference_creationDate_getset, filereference_creationDate_getset);
    o.init_property("creator", filereference_creator_getset, filereference_creator_getset);
    o.init_property("modificationDate", filereference_modificationDate_getset, filereference_modificationDate_getset);
    o.init_property("name", filereference_name_getset, filereference_name_getset);
    o.init_property("size", filereference_size_getset, filereference_size_getset);
    o.init_property("type", filereference_type_getset, filereference_type_getset);
}

static void
attachFileReferenceStaticProperties(as_object& /*o*/)
{
   
}

static as_value
filereference_addListener(const fn_call& /*fn*/)
{
    return as_value();
}

static as_value
filereference_browse(const fn_call& /*fn*/)
{
    return as_value();
}

static as_value
filereference_cancel(const fn_call& /*fn*/)
{
    return as_value();
}

static as_value
filereference_download(const fn_call& /*fn*/)
{
    return as_value();
}

static as_value
filereference_removeListener(const fn_call& /*fn*/)
{
    return as_value();
}

static as_value
filereference_upload(const fn_call& /*fn*/)
{
    return as_value();
}

static as_value
filereference_creationDate_getset(const fn_call& /*fn*/)
{
    return as_value();
}

static as_value
filereference_creator_getset(const fn_call& /*fn*/)
{
    return as_value();
}

static as_value
filereference_modificationDate_getset(const fn_call& /*fn*/)
{
    return as_value();
}

static as_value
filereference_name_getset(const fn_call& /*fn*/)
{
    return as_value();
}

static as_value
filereference_size_getset(const fn_call& /*fn*/)
{
    return as_value();
}

static as_value
filereference_type_getset(const fn_call& /*fn*/)
{
    return as_value();
}



as_value
filereference_ctor(const fn_call& fn)
{
    if (fn.nargs) {
        std::stringstream ss;
        fn.dump_args(ss);
        LOG_ONCE(
            log_unimpl("FileReference(%s): %s", ss.str(),
                _("arguments discarded"))
        );
    }

    return as_value();
}

// extern 
void
filereference_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, filereference_ctor,
            attachFileReferenceInterface, 
            attachFileReferenceStaticInterface, uri);
}

} // end of gnash namespace
