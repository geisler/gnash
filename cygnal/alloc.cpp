// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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
#include "config.h"
#endif

/// This defines thread safe new/delete operators
#include <new>
#include <boost/thread/mutex.hpp>

static boost::mutex mem_mutex;

// Wrap new in a mutex, because it is not thread safe.
void *
operator new (std::size_t bytes) throw (std::bad_alloc) {
    boost::mutex::scoped_lock lock(mem_mutex);
    void *ptr = malloc (bytes);
    return ptr;
}

// Wrap delete in a mutex, because it is not thread safe.
void
operator delete (void* vptr) throw () {
    boost::mutex::scoped_lock lock(mem_mutex);
    free (vptr);
}


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
