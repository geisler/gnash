// OutputProgressEvent_as.hx:  ActionScript 3 "OutputProgressEvent" class, for Gnash.
//
// Generated by gen-as3.sh on: 20090515 by "rob". Remove this
// after any hand editing loosing changes.
//
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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

// This test case must be processed by CPP before compiling to include the
//  DejaGnu.hx header file for the testing framework support.

#if flash9
import flash.events.OutputProgressEvent;
import flash.display.MovieClip;
#else
import flash.OutputProgressEvent;
import flash.MovieClip;
#end
import flash.Lib;
import Type;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class OutputProgressEvent_as {
    static function main() {
        var x1:OutputProgressEvent = new OutputProgressEvent();

        // Make sure we actually get a valid class        
        if (x1 != null) {
            DejaGnu.pass("OutputProgressEvent class exists");
        } else {
            DejaGnu.fail("OutputProgressEvent class doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (x1.bytesPending == 0) {
	    DejaGnu.pass("OutputProgressEvent.bytesPending property exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent.bytesPending property doesn't exist");
	}
	if (x1.bytesTotal == 0) {
	    DejaGnu.pass("OutputProgressEvent.bytesTotal property exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent.bytesTotal property doesn't exist");
	}

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (x1.OutputProgressEvent == 0) {
	    DejaGnu.pass("OutputProgressEvent::OutputProgressEvent() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::OutputProgressEvent() method doesn't exist");
	}
	if (x1.clone == Event) {
	    DejaGnu.pass("OutputProgressEvent::clone() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::clone() method doesn't exist");
	}
	if (x1.toString == null) {
	    DejaGnu.pass("OutputProgressEvent::toString() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::toString() method doesn't exist");
	}
	if (x1.ACTIVATE == null) {
	    DejaGnu.pass("OutputProgressEvent::ACTIVATE() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::ACTIVATE() method doesn't exist");
	}
	if (x1.ADDED == null) {
	    DejaGnu.pass("OutputProgressEvent::ADDED() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::ADDED() method doesn't exist");
	}
	if (x1.ADDED == TO) {
	    DejaGnu.pass("OutputProgressEvent::ADDED() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::ADDED() method doesn't exist");
	}
	if (x1.CANCEL == null) {
	    DejaGnu.pass("OutputProgressEvent::CANCEL() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::CANCEL() method doesn't exist");
	}
	if (x1.CHANGE == null) {
	    DejaGnu.pass("OutputProgressEvent::CHANGE() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::CHANGE() method doesn't exist");
	}
	if (x1.CLOSE == null) {
	    DejaGnu.pass("OutputProgressEvent::CLOSE() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::CLOSE() method doesn't exist");
	}
	if (x1.CLOSING == null) {
	    DejaGnu.pass("OutputProgressEvent::CLOSING() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::CLOSING() method doesn't exist");
	}
	if (x1.COMPLETE == null) {
	    DejaGnu.pass("OutputProgressEvent::COMPLETE() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::COMPLETE() method doesn't exist");
	}
	if (x1.CONNECT == null) {
	    DejaGnu.pass("OutputProgressEvent::CONNECT() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::CONNECT() method doesn't exist");
	}
	if (x1.DEACTIVATE == null) {
	    DejaGnu.pass("OutputProgressEvent::DEACTIVATE() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::DEACTIVATE() method doesn't exist");
	}
	if (x1.DISPLAYING == null) {
	    DejaGnu.pass("OutputProgressEvent::DISPLAYING() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::DISPLAYING() method doesn't exist");
	}
	if (x1.ENTER == FRAME) {
	    DejaGnu.pass("OutputProgressEvent::ENTER() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::ENTER() method doesn't exist");
	}
	if (x1.EXITING == null) {
	    DejaGnu.pass("OutputProgressEvent::EXITING() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::EXITING() method doesn't exist");
	}
	if (x1.FULLSCREEN == null) {
	    DejaGnu.pass("OutputProgressEvent::FULLSCREEN() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::FULLSCREEN() method doesn't exist");
	}
	if (x1.HTML == BOUNDS) {
	    DejaGnu.pass("OutputProgressEvent::HTML() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::HTML() method doesn't exist");
	}
	if (x1.HTML == DOM) {
	    DejaGnu.pass("OutputProgressEvent::HTML() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::HTML() method doesn't exist");
	}
	if (x1.HTML == RENDER) {
	    DejaGnu.pass("OutputProgressEvent::HTML() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::HTML() method doesn't exist");
	}
	if (x1.ID3 == null) {
	    DejaGnu.pass("OutputProgressEvent::ID3() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::ID3() method doesn't exist");
	}
	if (x1.INIT == null) {
	    DejaGnu.pass("OutputProgressEvent::INIT() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::INIT() method doesn't exist");
	}
	if (x1.LOCATION == CHANGE) {
	    DejaGnu.pass("OutputProgressEvent::LOCATION() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::LOCATION() method doesn't exist");
	}
	if (x1.MOUSE == LEAVE) {
	    DejaGnu.pass("OutputProgressEvent::MOUSE() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::MOUSE() method doesn't exist");
	}
	if (x1.NETWORK == CHANGE) {
	    DejaGnu.pass("OutputProgressEvent::NETWORK() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::NETWORK() method doesn't exist");
	}
	if (x1.OPEN == null) {
	    DejaGnu.pass("OutputProgressEvent::OPEN() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::OPEN() method doesn't exist");
	}
	if (x1.OUTPUT == PROGRESS) {
	    DejaGnu.pass("OutputProgressEvent::OUTPUT() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::OUTPUT() method doesn't exist");
	}
	if (x1.REMOVED == null) {
	    DejaGnu.pass("OutputProgressEvent::REMOVED() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::REMOVED() method doesn't exist");
	}
	if (x1.REMOVED == FROM) {
	    DejaGnu.pass("OutputProgressEvent::REMOVED() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::REMOVED() method doesn't exist");
	}
	if (x1.RENDER == null) {
	    DejaGnu.pass("OutputProgressEvent::RENDER() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::RENDER() method doesn't exist");
	}
	if (x1.RESIZE == null) {
	    DejaGnu.pass("OutputProgressEvent::RESIZE() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::RESIZE() method doesn't exist");
	}
	if (x1.SCROLL == null) {
	    DejaGnu.pass("OutputProgressEvent::SCROLL() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::SCROLL() method doesn't exist");
	}
	if (x1.SELECT == null) {
	    DejaGnu.pass("OutputProgressEvent::SELECT() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::SELECT() method doesn't exist");
	}
	if (x1.SOUND == COMPLETE) {
	    DejaGnu.pass("OutputProgressEvent::SOUND() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::SOUND() method doesn't exist");
	}
	if (x1.TAB == CHILDREN) {
	    DejaGnu.pass("OutputProgressEvent::TAB() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::TAB() method doesn't exist");
	}
	if (x1.TAB == ENABLED) {
	    DejaGnu.pass("OutputProgressEvent::TAB() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::TAB() method doesn't exist");
	}
	if (x1.TAB == INDEX) {
	    DejaGnu.pass("OutputProgressEvent::TAB() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::TAB() method doesn't exist");
	}
	if (x1.UNLOAD == null) {
	    DejaGnu.pass("OutputProgressEvent::UNLOAD() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::UNLOAD() method doesn't exist");
	}
	if (x1.USER == IDLE) {
	    DejaGnu.pass("OutputProgressEvent::USER() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::USER() method doesn't exist");
	}
	if (x1.USER == PRESENT) {
	    DejaGnu.pass("OutputProgressEvent::USER() method exists");
	} else {
	    DejaGnu.fail("OutputProgressEvent::USER() method doesn't exist");
	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

