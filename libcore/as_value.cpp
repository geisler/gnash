// as_value.cpp:  ActionScript values, for Gnash.
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

#include "smart_ptr.h" // GNASH_USE_GC
#include "as_value.h"
#include "as_object.h"
#include "as_function.h" // for as_function
#include "MovieClip.h" // for DISPLAYOBJECT values
#include "DisplayObject.h" // for DISPLAYOBJECT values
#include "as_environment.h" // for DISPLAYOBJECT values
#include "VM.h" // for DISPLAYOBJECT values
#include "movie_root.h" // for DISPLAYOBJECT values
#include "utility.h" // for typeName() 
#include "GnashNumeric.h"
#include "namedStrings.h"
#include "element.h"
#include "GnashException.h"
#include "amf.h"
#include "Array_as.h"
#include "Date_as.h" // for Date type (readAMF0)
#include "SimpleBuffer.h"
#include "StringPredicates.h"
#include "Global_as.h"
#include "String_as.h"

#include <boost/shared_ptr.hpp>
#include <cmath> 
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <locale>
#include <sstream>
#include <iomanip>
#include <string>

// Define the macro below to make abstract equality operator verbose
//#define GNASH_DEBUG_EQUALITY 1

// Define the macro below to make to_primitive verbose
//#define GNASH_DEBUG_CONVERSION_TO_PRIMITIVE 1

// Define this macro to make soft references activity verbose
#define GNASH_DEBUG_SOFT_REFERENCES

// Define this macro to make AMF parsing verbose
//#define GNASH_DEBUG_AMF_DESERIALIZE 1

// Define this macto to make AMF writing verbose
// #define GNASH_DEBUG_AMF_SERIALIZE 1

namespace gnash {

namespace {

/// Returns a member only if it is an object.
inline bool
findMethod(as_object& obj, string_table::key m, as_value& ret)
{
    return obj.get_member(m, &ret) && ret.is_object();
}

inline boost::uint16_t
readNetworkShort(const boost::uint8_t* buf) {
	boost::uint16_t s = buf[0] << 8 | buf[1];
	return s;
}

inline boost::uint32_t
readNetworkLong(const boost::uint8_t* buf) {
	boost::uint32_t s = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
	return s;
}

/// Truncates a double to a 32-bit unsigned int.
//
/// In fact, it is a 32-bit unsigned int with an additional sign, cast
/// to an unsigned int. Not sure what the sense is, but that's how it works:
//
/// 0xffffffff is interpreted as -1, -0xffffffff as 1.
boost::int32_t
truncateToInt(double d)
{
    if (d < 0) {   
	    return - static_cast<boost::uint32_t>(std::fmod(-d, 4294967296.0));
    }
	
    return static_cast<boost::uint32_t>(std::fmod(d, 4294967296.0));
}

enum Base
{
    BASE_OCT,
    BASE_HEX
};


/// Converts a string to a uint32_t cast to an int32_t.
//
/// @param whole    When true, any string that isn't wholly valid is rejected.
/// @param base     The base (8 or 16) to use.
/// @param s        The string to parse.
/// @return         The converted number.
boost::int32_t
parsePositiveInt(const std::string& s, Base base, bool whole = true)
{

    std::istringstream is(s);
    boost::uint32_t target;

    switch (base)
    {
        case BASE_OCT:
            is >> std::oct;
            break;
        case BASE_HEX:
            is >> std::hex;
            break;
    }

    char c;

    // If the cast fails, or if the whole string must be convertible and
    // some DisplayObjects are left, throw an exception.
    if (!(is >> target) || (whole && is.get(c))) {
        throw boost::bad_lexical_cast();
    }

    return target;
}

// This class is used to iterate through all the properties of an AS object,
// so we can change them to children of an AMF0 element.
class PropsSerializer : public AbstractPropertyVisitor
{

public:

    PropsSerializer(amf::Element& el, VM& vm)
        :
        _obj(el),
	    _st(vm.getStringTable())
	{}
    
    bool accept(const ObjectURI& uri, const as_value& val) 
    {

        // We are not taking account of the namespace.
        const string_table::key key = getName(uri);

        // Test conducted with AMFPHP:
        // '__proto__' and 'constructor' members
        // of an object don't get back from an 'echo-service'.
        // Dunno if they are not serialized or just not sent back.
        // A '__constructor__' member gets back, but only if 
        // not a function. Actually no function gets back.
        // 
        if (key == NSV::PROP_uuPROTOuu || key == NSV::PROP_CONSTRUCTOR)
        {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(" skip serialization of specially-named property %s",
                    _st.value(key));
#endif
            return true;
        }

        amf::AMF amf;
        boost::shared_ptr<amf::Element> el;
    
        const std::string& name = _st.value(key);

        if (val.is_string()) {
            std::string str;
            if (!val.is_undefined()) {
                str = val.to_string();
            }
            el.reset(new amf::Element(name, str));
        } else if (val.is_bool()) {
            bool flag = val.to_bool();
            el.reset(new amf::Element(name, flag));
        } else if (val.is_object()) {
//            el.reset(new amf::Element(name, flag));
        } else if (val.is_null()) {
	    boost::shared_ptr<amf::Element> tmpel(new amf::Element);
	    tmpel->setName(name);
	    tmpel->makeNull();
            el = tmpel;
        } else if (val.is_undefined()) {
	    boost::shared_ptr<amf::Element> tmpel(new amf::Element);
	    tmpel->setName(name);
	    tmpel->makeUndefined();
            el = tmpel;
        } else if (val.is_number()) { 
            double dub;
            if (val.is_undefined()) {
                dub = 0.0;
            } else {
                dub = val.to_number();
            }
            el.reset(new amf::Element(name, dub));
        }
    
        if (el) {
            _obj.addProperty(el);
        }
        return true;
    }

private:

    amf::Element& _obj;
    string_table& _st;

};

} // anonymous namespace

/// Class used to serialize properties of an object to a buffer
class PropsBufSerializer : public AbstractPropertyVisitor
{

    typedef std::map<as_object*, size_t> PropertyOffsets;

public:
    PropsBufSerializer(SimpleBuffer& buf, VM& vm,
            PropertyOffsets& offsetTable, bool allowStrict)
        :
        _allowStrict(allowStrict),
        _buf(buf),
        _vm(vm),
        _st(vm.getStringTable()),
        _offsetTable(offsetTable),
        _error(false)
	{}
    
    bool success() const { return !_error; }

    bool accept(const ObjectURI& uri, const as_value& val) 
    {
        if ( _error ) return true;

        // Tested with SharedObject and AMFPHP
        if (val.is_function())
        {
            log_debug("AMF0: skip serialization of FUNCTION property");
            return true;
        }

        const string_table::key key = getName(uri);

        // Test conducted with AMFPHP:
        // '__proto__' and 'constructor' members
        // of an object don't get back from an 'echo-service'.
        // Dunno if they are not serialized or just not sent back.
        // A '__constructor__' member gets back, but only if 
        // not a function. Actually no function gets back.
        if (key == NSV::PROP_uuPROTOuu || key == NSV::PROP_CONSTRUCTOR)
        {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(" skip serialization of specially-named property %s",
                    _st.value(key));
#endif
            return true;
        }

        // write property name
        const std::string& name = _st.value(key);
#ifdef GNASH_DEBUG_AMF_SERIALIZE
        log_debug(" serializing property %s", name);
#endif
        boost::uint16_t namelen = name.size();
        _buf.appendNetworkShort(namelen);
        _buf.append(name.c_str(), namelen);
        if (!val.writeAMF0(_buf, _offsetTable, _vm, _allowStrict)) {
            log_error("Problems serializing an object's member");
            _error = true;
        }
        return true;
    }
private:

    bool _allowStrict;
    SimpleBuffer& _buf;
    VM& _vm;
    string_table& _st;
    PropertyOffsets& _offsetTable;
    mutable bool _error;

};
    
// Conversion to const std::string&.
std::string
as_value::to_string(int version) const
{
	switch (_type)
	{

		case STRING:
			return getStr();

		case DISPLAYOBJECT:
		{
			const CharacterProxy& sp = getCharacterProxy();
			if (!sp.get()) return "";
            return sp.getTarget();
		}

		case NUMBER:
		{
			const double d = getNum();
			return doubleToString(d);
		}

		case UNDEFINED: 
		    if (version <= 6) return "";
			return "undefined";

		case NULLTYPE:
			return "null";

		case BOOLEAN:
			return getBool() ? "true" : "false";

		case OBJECT:
		{
            as_object* obj = getObj();
            String_as* s;
            if (isNativeType(obj, s)) return s->value();

			try {
				as_value ret = to_primitive(STRING);
				// This additional is_string test is NOT compliant with ECMA-262
				// specification, but seems required for compatibility with the
				// reference player.
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
				log_debug(" %s.to_primitive(STRING) returned %s", *this, ret);
#endif
				if ( ret.is_string() ) return ret.to_string();
			}
			catch (ActionTypeError& e) {
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
				log_debug(_("to_primitive(%s, STRING) threw an ActionTypeError %s"),
						*this, e.what());
#endif
			}

			if (_type == OBJECT) {
                return is_function() ? "[type Function]" :
                                       "[type Object]";
            }

		}

		default:
			return "[exception]";
    }
    
}

/// This is only used in AVM2.
primitive_types
as_value::ptype() const
{

	switch (_type)
	{
        case STRING:
            return PTYPE_STRING;
        case NUMBER: 
        case UNDEFINED:
        case NULLTYPE:
        case DISPLAYOBJECT:
        case OBJECT:
            return PTYPE_NUMBER;
        case BOOLEAN:
            return PTYPE_BOOLEAN;
        default:
            break;
    }
	return PTYPE_NUMBER;
}

as_value::AsType
as_value::defaultPrimitive(int version) const
{
	if (_type == OBJECT && version > 5) {
        Date_as* d;
        if (isNativeType(getObj(), d)) return STRING;
    }
    return NUMBER;
}

// Conversion to primitive value.
as_value
as_value::to_primitive(AsType hint) const
{
	if (_type != OBJECT) return *this; 

#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
	log_debug("to_primitive(%s)", hint==NUMBER ? "NUMBER" : "STRING");
#endif 

	// TODO: implement as_object::DefaultValue (ECMA-262 - 8.6.2.6)

	as_value method;
	as_object* obj(0);

	if (hint == NUMBER) {

		if (_type == DISPLAYOBJECT) return as_value(NaN);

        assert(_type == OBJECT);
		obj = getObj();

		if (!findMethod(*obj, NSV::PROP_VALUE_OF, method)) {
            // Returning undefined here instead of throwing
            // a TypeError passes tests in actionscript.all/Object.as
            // and many swfdec tests, with no new failures (though
            // perhaps we aren't testing enough).
            return as_value();
		}
	}
	else {
		assert(hint == STRING);

		if (_type == DISPLAYOBJECT) return getCharacterProxy().getTarget();
        assert(_type == OBJECT);
		obj = getObj();

		// @@ Moock says, "the value that results from
		// calling toString() on the object".
		//
		// When the toString() method doesn't exist, or
		// doesn't return a valid number, the default
		// text representation for that object is used
		// instead.
		//
		if (!findMethod(*obj, NSV::PROP_TO_STRING, method) &&
                !findMethod(*obj, NSV::PROP_VALUE_OF, method)) {
				throw ActionTypeError();
        }
	}

	assert(obj);

	as_environment env(getVM(*obj));
    fn_call::Args args;
	as_value ret = invoke(method, env, obj, args);

#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
	log_debug("to_primitive: method call returned %s", ret);
#endif

	if (ret._type == OBJECT) {
		throw ActionTypeError();
	}
	return ret;
}

double
as_value::to_number() const
{

    const int swfversion = VM::get().getSWFVersion();

    switch (_type)
    {
        case STRING:
        {
            const std::string& s = getStr();
            if ( s.empty() ) {
                return swfversion >= 5 ? NaN : 0.0;
            }
            
            if (swfversion <= 4)
            {
                // For SWF4, any valid number before non-numerical
                // DisplayObjects is returned, including exponent, positive
                // and negative signs and whitespace before.
                double d = 0;
                std::istringstream is(s);
                is >> d;
                return d;
            }

            try {

                if (swfversion > 5)
                {
                    double d;
                    // Will throw if invalid.
                    if (parseNonDecimalInt(s, d)) return d;
                }

                // @@ Moock says the rule here is: if the
                // string is a valid float literal, then it
                // gets converted; otherwise it is set to NaN.
                // Valid for SWF5 and above.
                //
                // boost::lexical_cast is remarkably inflexible and 
                // fails for anything that has non-numerical DisplayObjects.
                // Fortunately, actionscript is equally inflexible.
                std::string::size_type pos;
                if ((pos = s.find_first_not_of(" \r\n\t")) 
                        == std::string::npos) {
                    return NaN;
                }

                return boost::lexical_cast<double>(s.substr(pos));
 
            }
            catch (boost::bad_lexical_cast&) {
                // There is no standard textual representation of infinity
                // in the C++ standard, so our conversion function an
                // exception for 'inf', just like for any other
                // non-numerical text. This is correct behaviour.
                return NaN;
            }
        }

        case NULLTYPE:
        case UNDEFINED: 
        {
            // Evan: from my tests
            // Martin: FlashPlayer6 gives 0; FP9 gives NaN.
            return ( swfversion >= 7 ? NaN : 0 );
        }

        case BOOLEAN: 
            // Evan: from my tests
            // Martin: confirmed
            return getBool() ? 1 : 0;

        case NUMBER:
            return getNum();

        case OBJECT:
        {
            // @@ Moock says the result here should be
            // "the return value of the object's valueOf()
            // method".
            //
            // Arrays and Movieclips should return NaN.
            try
            {
                as_value ret = to_primitive(NUMBER);
                return ret.to_number();
            }
            catch (ActionTypeError& e)
            {
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
                log_debug(_("to_primitive(%s, NUMBER) threw an "
                            "ActionTypeError %s"), *this, e.what());
#endif
                if (is_function() && swfversion < 6) {
                    return 0;
                }
                
                return NaN;
            }
        }

        case DISPLAYOBJECT:
        {
            // This is tested, no valueOf is going
            // to be invoked for movieclips.
            return NaN; 
        }

        default:
            // Other object types should return NaN.
            return NaN;
    }
}

boost::shared_ptr<amf::Element>
as_value::to_element() const
{
    VM& vm = VM::get();
    //int swfVersion = vm.getSWFVersion();
    boost::shared_ptr<amf::Element> el ( new amf::Element );
    as_object* ptr = to_object(*vm.getGlobal());

    switch (_type) {
      case UNDEFINED:
	  el->makeUndefined();
	  break;
      case NULLTYPE:
	  el->makeNull();
	  break;
      case BOOLEAN:
	  el->makeBoolean(getBool());
	  break;
      case  STRING:
	  el->makeString(getStr());
	  break;
      case NUMBER:
	  el->makeNumber(getNum());
	  break;
      case OBJECT:
      {
          if (is_function()) break;
          el->makeObject();
          PropsSerializer props(*el, vm);
          ptr->visitProperties<Exists>(props);
	  break;
      }
      case DISPLAYOBJECT:
	  log_unimpl("Converting a Movie Clip to an element is not supported");
	  break;
      default:
	  break;
    }

    return el;
}

// Conversion to boolean 
bool
as_value::to_bool() const
{
    const int version = VM::get().getSWFVersion();
    switch (_type)
    {
		case STRING:
		{
            if (version >= 7) return !getStr().empty();
			const double num = to_number();
			return num && !isNaN(num);
		}
		case NUMBER:
		{
			const double d = getNum();
            // see testsuite/swfdec/if-6.swf
			return d && ! isNaN(d);
		}
		case BOOLEAN:
			return getBool();
		case OBJECT:
			return true;
		case DISPLAYOBJECT:
			return true;
		default:
			assert(_type == UNDEFINED || _type == NULLTYPE || is_exception());
			return false;
	}
}

// Return value as an object.
as_object*
as_value::to_object(Global_as& global) const
{
	switch (_type)
	{
		case OBJECT:
			return getObj();

		case DISPLAYOBJECT:
			return getObject(toDisplayObject());

		case STRING:
			return global.createString(getStr());

		case NUMBER:
			return global.createNumber(getNum());

		case BOOLEAN:
			return global.createBoolean(getBool());

		default:
			// Invalid to convert exceptions.
			return NULL;
	}
}

MovieClip*
as_value::toMovieClip(bool allowUnloaded) const
{
	if ( _type != DISPLAYOBJECT ) return 0;

	DisplayObject *ch = getCharacter(allowUnloaded);
	if ( ! ch ) return 0;
	return ch->to_movie();
}

DisplayObject*
as_value::toDisplayObject(bool allowUnloaded) const
{
	if (_type != DISPLAYOBJECT) return NULL;
	return getCharacter(allowUnloaded);
}

// Return value as an ActionScript function.  Returns NULL if value is
// not an ActionScript function.
as_function*
as_value::to_function() const
{
    if (_type == OBJECT) {
	    return getObj()->to_function();
    }

    return 0;
}

void
as_value::set_undefined()
{
	_type = UNDEFINED;
	_value = boost::blank();
}

void
as_value::set_null()
{
	_type = NULLTYPE;
	_value = boost::blank();
}

void
as_value::set_as_object(as_object* obj)
{
	if ( ! obj )
	{
		set_null();
		return;
	}
    if (obj->displayObject()) {
        // The static cast is fine as long as the as_object is genuinely
        // a DisplayObject.
        _type = DISPLAYOBJECT;
        _value = CharacterProxy(obj->displayObject());
		return;
	}

	if (_type != OBJECT || getObj() != obj) {
		_type = OBJECT;
		_value = obj;
	}
}

bool
as_value::equals(const as_value& v) const
{
    // Comments starting with numbers refer to the ECMA-262 document

    int SWFVersion = VM::get().getSWFVersion();

    bool this_nulltype = (_type == UNDEFINED || _type == NULLTYPE);
    bool v_nulltype = (v._type == UNDEFINED || v._type == NULLTYPE);

    // It seems like functions are considered the same as a NULL type
    // in SWF5 (and I hope below, didn't check)
    if (SWFVersion < 6) {
        if (is_function()) this_nulltype = true;
        if (v.is_function()) v_nulltype = true;
    }

    if (this_nulltype || v_nulltype) {
#ifdef GNASH_DEBUG_EQUALITY
       log_debug(" one of the two things is undefined or null");
#endif
        return this_nulltype == v_nulltype;
    }

    bool obj_or_func = (_type == OBJECT);
    bool v_obj_or_func = (v._type == OBJECT);

    /// Compare to same type
    if (obj_or_func && v_obj_or_func) {
        return boost::get<as_object*>(_value) ==
            boost::get<as_object*>(v._value); 
    }

    if (_type == v._type) return equalsSameType(v);

    // 16. If Type(x) is Number and Type(y) is String,
    //    return the result of the comparison x == ToNumber(y).
    if (_type == NUMBER && v._type == STRING) {
        const double n = v.to_number();
        if (!isFinite(n)) return false;
        return equalsSameType(n);
    }

    // 17. If Type(x) is String and Type(y) is Number,
    //     return the result of the comparison ToNumber(x) == y.
    if (v._type == NUMBER && _type == STRING) {
        const double n = to_number();
        if (!isFinite(n)) return false;
        return v.equalsSameType(n); 
    }

    // 18. If Type(x) is Boolean, return the result of the comparison ToNumber(x) == y.
    if (_type == BOOLEAN) {
        return as_value(to_number()).equals(v); 
    }

    // 19. If Type(y) is Boolean, return the result of the comparison x == ToNumber(y).
    if (v._type == BOOLEAN) {
        return as_value(v.to_number()).equals(*this); 
    }

    // 20. If Type(x) is either String or Number and Type(y) is Object,
    //     return the result of the comparison x == ToPrimitive(y).
    if ((_type == STRING || _type == NUMBER) && (v._type == OBJECT))
    {
        // convert this value to a primitive and recurse
        try {
            as_value v2 = v.to_primitive(v.defaultPrimitive(SWFVersion)); 
            if (v.strictly_equals(v2)) return false;
#ifdef GNASH_DEBUG_EQUALITY
            log_debug(" 20: convertion to primitive : %s -> %s", v, v2);
#endif
            return equals(v2);
        }
        catch (ActionTypeError& e) {
#ifdef GNASH_DEBUG_EQUALITY
            log_debug(" %s.to_primitive() threw an ActionTypeError %s", v,
                    e.what());
#endif
            return false; 
        }
    }

    // 21. If Type(x) is Object and Type(y) is either String or Number,
    //    return the result of the comparison ToPrimitive(x) == y.
    if ((v._type == STRING || v._type == NUMBER) && (_type == OBJECT)) {
        // convert this value to a primitive and recurse
        try {
            // Date objects default to primitive type STRING from SWF6 up,
            // but we always prefer valueOf to toString in this case.
        	as_value v2 = to_primitive(NUMBER); 
            if (strictly_equals(v2)) return false;
#ifdef GNASH_DEBUG_EQUALITY
            log_debug(" 21: convertion to primitive : %s -> %s", *this, v2);
#endif
            return v2.equals(v);
        }
        catch (ActionTypeError& e) {
#ifdef GNASH_DEBUG_EQUALITY
            log_debug(" %s.to_primitive() threw an ActionTypeError %s",
                    *this, e.what());
#endif
            return false; 
        }
    }

#ifdef GNASH_DEBUG_EQUALITY
	// Both operands are objects (OBJECT,DISPLAYOBJECT)
	if (!is_object() || !v.is_object()) {
		log_debug("Equals(%s,%s)", *this, v);
	}
#endif

    // If any of the two converts to a primitive, we recurse

	as_value p = *this;
	as_value vp = v;

    bool converted(false);

	try {
		p = to_primitive(p.defaultPrimitive(SWFVersion)); 
		if (!strictly_equals(p)) converted = true;
#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" conversion to primitive (this): %s -> %s", *this, p);
#endif
	}
	catch (ActionTypeError& e) {
#ifdef GNASH_DEBUG_CONVERSION_TO_PRIMITIVE 
		log_debug(" %s.to_primitive() threw an ActionTypeError %s",
			*this, e.what());
#endif
	}

	try {
		vp = v.to_primitive(v.defaultPrimitive(SWFVersion)); 
		if (!v.strictly_equals(vp)) converted = true;
#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" conversion to primitive (that): %s -> %s", v, vp);
#endif
	}
	catch (ActionTypeError& e) {
#ifdef GNASH_DEBUG_CONVERSION_TO_PRIMITIVE 
		log_debug(" %s.to_primitive() threw an ActionTypeError %s",
			v, e.what());
#endif
	}

	if (converted)
	{
#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" some conversion took place, recursing");
#endif
		return p.equals(vp);
	}
	else {
#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" no conversion took place, returning false");
#endif
		return false;
	}


}
	
// Sets *this to this string plus the given string.
void
as_value::string_concat(const std::string& str)
{
    std::string currVal = to_string();
    _type = STRING;
    _value = currVal + str;
}

const char*
as_value::typeOf() const
{
	switch (_type)
	{
		case UNDEFINED:
			return "undefined"; 

		case STRING:
			return "string";

		case NUMBER:
			return "number";

		case BOOLEAN:
			return "boolean";

		case OBJECT:
            return is_function() ? "function" : "object";

		case DISPLAYOBJECT:
		{
			DisplayObject* ch = getCharacter();
			if ( ! ch ) return "movieclip"; // dangling
			if ( ch->to_movie() ) return "movieclip"; // bound to movieclip
			return "object"; // bound to some other DisplayObject
		}

		case NULLTYPE:
			return "null";

		default:
			if (is_exception()) return "exception";
            std::abort();
			return 0;
	}
}

bool
as_value::equalsSameType(const as_value& v) const
{
	assert(_type == v._type);

	switch (_type)
	{
		case UNDEFINED:
		case NULLTYPE:
			return true;

		case OBJECT:
		case BOOLEAN:
		case STRING:
			return _value == v._value;

		case DISPLAYOBJECT:
			return toDisplayObject() == v.toDisplayObject(); 

		case NUMBER:
		{
			const double a = getNum();
			const double b = v.getNum();
			if (isNaN(a) && isNaN(b)) return true;
			return a == b;
		}
		default:
			if (is_exception()) return false; 

	}
    std::abort();
	return false;
}

bool
as_value::strictly_equals(const as_value& v) const
{
	if ( _type != v._type ) return false;
	return equalsSameType(v);
}

std::string
as_value::toDebugString() const
{
    boost::format ret;

	switch (_type)
	{
		case UNDEFINED:
			return "[undefined]";
		case NULLTYPE:
			return "[null]";
		case BOOLEAN:
		    ret = boost::format("[bool:%s]") % (getBool() ? "true" : "false");
            return ret.str();
		case OBJECT:
		{
			as_object* obj = getObj();
			ret = boost::format("[object(%s):%p]") % typeName(*obj) %
                                              static_cast<void*>(obj);
			return ret.str();
		}
		case STRING:
			return "[string:" + getStr() + "]";
		case NUMBER:
		{
			std::stringstream stream;
			stream << getNum();
			return "[number:" + stream.str() + "]";
		}
		case DISPLAYOBJECT:
		{
			const CharacterProxy& sp = getCharacterProxy();
			if (sp.isDangling()) {
				DisplayObject* rebound = sp.get();
				if (rebound) {
				    ret = boost::format("[rebound %s(%s):%p]") % 
						typeName(*rebound) % sp.getTarget() %
						static_cast<void*>(rebound);
				}
				else {
				    ret = boost::format("[dangling DisplayObject:%s]") % 
					    sp.getTarget();
				}
			}
			else {
				DisplayObject* ch = sp.get();
				ret = boost::format("[%s(%s):%p]") % typeName(*ch) %
				                sp.getTarget() % static_cast<void*>(ch);
			}
			return ret.str();
		}
		default:
			if (is_exception()) return "[exception]";
            std::abort();
	}
}

void
as_value::operator=(const as_value& v)
{
	_type = v._type;
	_value = v._value;
}

void
as_value::setReachable() const
{
	switch (_type)
	{
		case OBJECT:
		{
			as_object* op = getObj();
			if (op) op->setReachable();
			break;
		}
		case DISPLAYOBJECT:
		{
			CharacterProxy sp = getCharacterProxy();
			sp.setReachable();
			break;
		}
		default: break;
	}
}

as_object*
as_value::getObj() const
{
	assert(_type == OBJECT);
	return boost::get<as_object*>(_value);
}

CharacterProxy
as_value::getCharacterProxy() const
{
	assert(_type == DISPLAYOBJECT);
	return boost::get<CharacterProxy>(_value);
}

DisplayObject*
as_value::getCharacter(bool allowUnloaded) const
{
	return getCharacterProxy().get(allowUnloaded);
}

void
as_value::set_string(const std::string& str)
{
	_type = STRING;
	_value = str;
}

void
as_value::set_double(double val)
{
	_type = NUMBER;
	_value = val;
}

void
as_value::set_bool(bool val)
{
	_type = BOOLEAN;
	_value = val;
}

as_value::as_value()
	:
	_type(UNDEFINED),
	_value(boost::blank())
{
}

as_value::as_value(const as_value& v)
	:
	_type(v._type),
	_value(v._value)
{
}

as_value::as_value(const char* str)
	:
	_type(STRING),
	_value(std::string(str))
{
}

as_value::as_value(const std::string& str)
	:
	_type(STRING),
	_value(str)
{
}

as_value::as_value(double num)
	:
	_type(NUMBER),
	_value(num)
{
}

as_value::as_value(as_object* obj)
	:
	_type(UNDEFINED)
{
	set_as_object(obj);
}

bool
as_value::is_function() const
{
    return _type == OBJECT && getObj()->to_function();
}

// Pass pointer to buffer and pointer to end of buffer. Buffer is raw AMF
// encoded data. Must start with a type byte unless third parameter is set.
//
// On success, sets the given as_value and returns true.
// On error (premature end of buffer, etc.) returns false and leaves the given
// as_value untouched.
//
// IF you pass a fourth parameter, it WILL NOT READ A TYPE BYTE, but use what
// you passed instead.
//
// The l-value you pass as the first parameter (buffer start) is updated to
// point just past the last byte parsed
//
// TODO restore first parameter on parse errors
//
static bool
amf0_read_value(const boost::uint8_t *&b, const boost::uint8_t *end, 
        as_value& ret, int inType, std::vector<as_object*>& objRefs, VM& vm)
{
	int amf_type;

	if (b > end) {
		return false;
	}
	
	if (inType != -1) {
		amf_type = inType;
	}
	else {
		if (b < end) {
			amf_type = *b; b += 1;
		} else {
			return false;
		}
	}

    Global_as& gl = *vm.getGlobal();

	switch (amf_type)
    {

		case amf::Element::BOOLEAN_AMF0:
		{
			bool val = *b; b += 1;
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
			log_debug("amf0 read bool: %d", val);
#endif
			ret.set_bool(val);
			return true;
		}

		case amf::Element::NUMBER_AMF0:
        {
			if (b + 8 > end) {
				log_error(_("AMF0 read: premature end of input reading Number type"));
				return false;
			}
			double dub;
            // TODO: may we avoid a copy and swapBytes call
            //       by bitshifting b[0] trough b[7] ?
            std::copy(b, b+8, (char*)&dub); b+=8; 
			amf::swapBytes(&dub, 8);
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
			log_debug("amf0 read double: %e", dub);
#endif
			ret.set_double(dub);
			return true;
        }

		case amf::Element::STRING_AMF0:
        {
			if (b + 2 > end) {
				log_error(_("AMF0 read: premature end of input reading String "
                            " type"));
				return false;
			}
            boost::uint16_t si = readNetworkShort(b); b += 2;
			if (b + si > end) {
				log_error(_("AMF0 read: premature end of input reading String "
                            "type"));
				return false;
			}

			{
				std::string str(reinterpret_cast<const char*>(b), si); b += si;
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
				log_debug("amf0 read string: %s", str);
#endif
				ret.set_string(str);
				return true;

			}
			break;
        }

		case amf::Element::LONG_STRING_AMF0:
        {
			if (b + 4 > end) {
				log_error(_("AMF0 read: premature end of input "
                            "reading Long String type"));
				return false;
			}
            boost::uint32_t si = readNetworkLong(b); b += 4;
			if (b + si > end) {
				log_error(_("AMF0 read: premature end of input "
                            "reading Long String type"));
				return false;
			}

			{
				std::string str(reinterpret_cast<const char*>(b), si); b += si;
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
				log_debug("amf0 read long string: %s", str);
#endif
				ret.set_string(str);
				return true;

			}
			break;
        }

		case amf::Element::STRICT_ARRAY_AMF0:
        {
            as_object* array = gl.createArray();
            objRefs.push_back(array);

            boost::uint32_t li = readNetworkLong(b); b += 4;
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
            log_debug("amf0 starting read of STRICT_ARRAY with %i elements", li);
#endif
            as_value arrayElement;
            for (size_t i = 0; i < li; ++i)
            {
                if ( ! amf0_read_value(b, end, arrayElement, -1,
                            objRefs, vm) )
                {
                    return false;
                }
                callMethod(array, NSV::PROP_PUSH, arrayElement);
            }

            ret.set_as_object(array);
            return true;
        }

		case amf::Element::ECMA_ARRAY_AMF0:
        {
            as_object* obj = gl.createArray();
            objRefs.push_back(obj);

            // set the value immediately, so if there's any problem parsing
            // (like premature end of buffer) we still get something.
            ret.set_as_object(obj);

            boost::uint32_t li = readNetworkLong(b); b += 4;
            
            log_debug("array size: %d", li);
            
            // the count specifies array size, so to have that even if none
            // of the members are indexed
            // if short, will be incremented everytime an indexed member is
            // found
            obj->set_member(NSV::PROP_LENGTH, li);

            // TODO: do boundary checking (if b >= end...)

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
            log_debug("amf0 starting read of ECMA_ARRAY with %i elements", li);
#endif
            as_value objectElement;
            string_table& st = vm.getStringTable();
            for (;;)
            {
                if ( b+2 >= end )
                {
                    log_error("MALFORMED SOL: premature end of ECMA_ARRAY "
                            "block");
                    break;
                }
                boost::uint16_t strlen = readNetworkShort(b); b+=2; 

                // end of ECMA_ARRAY is signalled by an empty string
                // followed by an OBJECT_END_AMF0 (0x09) byte
                if (!strlen) {
                    // expect an object terminator here
                    if (*b++ != amf::Element::OBJECT_END_AMF0) {
                        log_error("MALFORMED SOL: empty member name not "
                                "followed by OBJECT_END_AMF0 byte");
                    }
                    break;
                }

                std::string name(reinterpret_cast<const char*>(b), strlen);

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
                log_debug("amf0 ECMA_ARRAY prop name is %s", name);
#endif
                b += strlen;
                if (!amf0_read_value(b, end, objectElement, -1, objRefs, vm)) {
                    return false;
                }
                obj->set_member(st.find(name), objectElement);
            }

            return true;
        }

		case amf::Element::OBJECT_AMF0:
        {
            string_table& st = vm.getStringTable();

            as_object* obj = gl.createObject(); 

            // set the value immediately, so if there's any problem parsing
            // (like premature end of buffer) we still get something.
            ret.set_as_object(obj);

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
            log_debug("amf0 starting read of OBJECT");
#endif
            objRefs.push_back(obj);

            as_value tmp;
            std::string keyString;
            for(;;)
            {
                if ( ! amf0_read_value(b, end, tmp, 
                            amf::Element::STRING_AMF0, objRefs, vm) )
                {
                    return false;
                }
                keyString = tmp.to_string();

                if (keyString.empty()) {
                    if (b < end) {
                        b += 1; // AMF0 has a redundant "object end" byte
                    } else {
                        log_error("AMF buffer terminated just before "
                                "object end byte. continuing anyway.");
                    }
                    return true;
                }

                if (!amf0_read_value(b, end, tmp, -1, objRefs, vm)) {
                    return false;
                }
                obj->set_member(st.find(keyString), tmp);
            }
        }

		case amf::Element::UNDEFINED_AMF0:
        {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
				log_debug("readAMF0: undefined value");
#endif
				ret.set_undefined();
				return true;
        }

		case amf::Element::NULL_AMF0:
        {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
				log_debug("readAMF0: null value");
#endif
				ret.set_null();
				return true;
        }

		case amf::Element::REFERENCE_AMF0:
        {
            boost::uint16_t si = readNetworkShort(b); b += 2;
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
				log_debug("readAMF0: reference #%d", si);
#endif
                if ( si < 1 || si > objRefs.size() )
                {
                    log_error("readAMF0: invalid reference to object %d (%d known objects)", si, objRefs.size());
                    return false;
                }
                ret.set_as_object(objRefs[si-1]);
                return true;
        }

		case amf::Element::DATE_AMF0:
        {
			if (b + 8 > end) {
				log_error(_("AMF0 read: premature end of input reading Date "
                            "type"));
				return false;
			}
			double dub;
            // TODO: may we avoid a copy and swapBytes call
            //       by bitshifting b[0] trough b[7] ?
            std::copy(b, b+8, (char*)&dub); b+=8; 
			amf::swapBytes(&dub, 8);
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
			log_debug("amf0 read date: %e", dub);
#endif

            as_function* ctor = gl.getMember(NSV::CLASS_DATE).to_function();
            if (ctor) {
                fn_call::Args args;
                args += dub;
                ret.set_as_object(constructInstance(*ctor, as_environment(vm),
                            args));
            }

			if (b + 2 > end) {
				log_error(_("AMF0 read: premature end of input reading "
                            "timezone from Date type"));
				return false;
			}
            LOG_ONCE(log_unimpl("Timezone info from AMF0 encoded Date object "
                        "ignored"));
            b+=2;

			return true;
        }

		// TODO define other types (function, sprite, etc)
		default:
        {
			log_unimpl("AMF0 to as_value: unsupported type: %i", amf_type);
			return false;
        }
	}

	// this function was called with a zero-length buffer
	return false;
}

bool
as_value::readAMF0(const boost::uint8_t *&b, const boost::uint8_t *end,
        int inType, std::vector<as_object*>& objRefs, VM& vm)
{
	return amf0_read_value(b, end, *this, inType, objRefs, vm);
}

bool
as_value::writeAMF0(SimpleBuffer& buf, 
        std::map<as_object*, size_t>& offsetTable, VM& vm,
        bool allowStrict) const
{
    typedef std::map<as_object*, size_t> OffsetTable;

    assert (!is_exception());

    switch (_type)
    {
        default:
            log_unimpl(_("serialization of as_value of type %d"), _type);
            return false;

        case OBJECT:
        {
            if (is_function()) return false;
            as_object* obj = to_object(*vm.getGlobal());
            assert(obj);
            OffsetTable::iterator it = offsetTable.find(obj);
            if (it == offsetTable.end()) {
                size_t idx = offsetTable.size() + 1; // 1 for the first, etc...
                offsetTable[obj] = idx;
                
                Date_as* date;
                if (isNativeType(obj, date))
                {
                    double d = date->getTimeValue(); 
#ifdef GNASH_DEBUG_AMF_SERIALIZE
                    log_debug(_("writeAMF0: serializing date object "
                                "with index %d and value %g"), idx, d);
#endif
                    buf.appendByte(amf::Element::DATE_AMF0);

                    // This actually only swaps on little-endian machines
                    amf::swapBytes(&d, 8);
                    buf.append(&d, 8);

                    // This should be timezone
                    boost::uint16_t tz=0; 
                    buf.appendNetworkShort(tz);

                    return true;
                }

                if (obj->array()) {
                    string_table& st = vm.getStringTable();
                    const size_t len = arrayLength(*obj);
                    if (allowStrict) {
                        IsStrictArray s(st);
                        // Check if any non-hidden properties are non-numeric.
                        obj->visitProperties<IsEnumerable>(s);
                        if (s.strict()) {

#ifdef GNASH_DEBUG_AMF_SERIALIZE
                            log_debug(_("writeAMF0: serializing array of %d "
                                        "elements as STRICT_ARRAY (index %d)"),
                                        len, idx);
#endif
                            buf.appendByte(amf::Element::STRICT_ARRAY_AMF0);
                            buf.appendNetworkLong(len);

                            as_value elem;
                            for (size_t i = 0; i < len; ++i) {
                                elem = obj->getMember(arrayKey(st, i));
                                if (!elem.writeAMF0(buf, offsetTable, vm,
                                            allowStrict)) {
                                    log_error("Problems serializing strict array "
                                            "member %d=%s", i, elem);
                                    return false;
                                }
                            }
                            return true;
                        }
                    }

                    // A normal array.
#ifdef GNASH_DEBUG_AMF_SERIALIZE
                    log_debug(_("writeAMF0: serializing array of %d "
                                "elements as ECMA_ARRAY (index %d) "
                                "[allowStrict:%d, isStrict:%d]"),
                                len, idx, allowStrict, isStrict);
#endif
                    buf.appendByte(amf::Element::ECMA_ARRAY_AMF0);
                    buf.appendNetworkLong(len);
                }
                else {
                    // It's a simple object
#ifdef GNASH_DEBUG_AMF_SERIALIZE
                    log_debug(_("writeAMF0: serializing object (or function) "
                                "with index %d"), idx);
#endif
                    buf.appendByte(amf::Element::OBJECT_AMF0);
                }

                PropsBufSerializer props(buf, vm, offsetTable, allowStrict);
                obj->visitProperties<IsEnumerable>(props);
                if (!props.success()) {
                    log_error("Could not serialize object");
                    return false;
                }
                buf.appendNetworkShort(0);
                buf.appendByte(amf::Element::OBJECT_END_AMF0);
                return true;
            }
            size_t idx = it->second;
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(_("writeAMF0: serializing object (or function) "
                        "as reference to %d"), idx);
#endif
            buf.appendByte(amf::Element::REFERENCE_AMF0);
            buf.appendNetworkShort(idx);
            return true;
        }

        case STRING:
        {
            const std::string& str = getStr();
            size_t strlen = str.size();
            if ( strlen <= 65535 )
            {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
                log_debug(_("writeAMF0: serializing string '%s'"), str);
#endif
                buf.appendByte(amf::Element::STRING_AMF0);
                buf.appendNetworkShort(strlen);
                buf.append(str.c_str(), strlen);
            }
            else
            {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
                log_debug(_("writeAMF0: serializing long string '%s'"), str);
#endif
                buf.appendByte(amf::Element::LONG_STRING_AMF0);
                buf.appendNetworkLong(strlen);
                buf.append(str.c_str(), strlen);
            }
            return true;
        }

        case NUMBER:
        {
            double d = getNum();
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(_("writeAMF0: serializing number '%g'"), d);
#endif
            buf.appendByte(amf::Element::NUMBER_AMF0);
            amf::swapBytes(&d, 8); // this actually only swaps on little-endian machines
            buf.append(&d, 8);
            return true;
        }

        case DISPLAYOBJECT:
        {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(_("writeAMF0: serializing DISPLAYOBJECT (as undefined)"));
#endif
            // See misc-ming.all/SharedObjectTest.as
            buf.appendByte(amf::Element::UNDEFINED_AMF0);
            return true;
        }

        case NULLTYPE:
        {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(_("writeAMF0: serializing null"));
#endif
            buf.appendByte(amf::Element::NULL_AMF0);
            return true;
        }

        case UNDEFINED:
        {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(_("writeAMF0: serializing undefined"));
#endif
            buf.appendByte(amf::Element::UNDEFINED_AMF0);
            return true;
        }

        case BOOLEAN:
        {
            bool tf = getBool();
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(_("writeAMF0: serializing boolean '%s'"), tf);
#endif

            buf.appendByte(amf::Element::BOOLEAN_AMF0);
            if (tf) buf.appendByte(1);
            else buf.appendByte(0);

            return true;
        }
    }
}


boost::int32_t
toInt(const as_value& val) 
{
	const double d = val.to_number();

	if (!isFinite(d)) return 0;

    return truncateToInt(d);
}

bool
parseNonDecimalInt(const std::string& s, double& d, bool whole)
{
    const std::string::size_type slen = s.length();

    // "0#" would still be octal, but has the same value as a decimal.
    if (slen < 3) return false;

    bool negative = false;

    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        // The only legitimate place for a '-' is after 0x. If it's a
        // '+' we don't care, as it won't disturb the conversion.
        std::string::size_type start = 2;
        if (s[2] == '-') {
            negative = true;
            ++start;
        }
        d = parsePositiveInt(s.substr(start), BASE_HEX, whole);
        if (negative) d = -d;
        return true;
    }
    else if ((s[0] == '0' || ((s[0] == '-' || s[0] == '+') && s[1] == '0')) &&
            s.find_first_not_of("01234567", 1) == std::string::npos) {

        std::string::size_type start = 0;
        if (s[0] == '-') {
            negative = true;
            ++start;
        }
        d = parsePositiveInt(s.substr(start), BASE_OCT, whole);
        if (negative) d = -d;
        return true;
    }

    return false;

}

std::string
doubleToString(double val, int radix)
{
    // Examples:
    //
    // e.g. for 9*.1234567890123456789:
    // 9999.12345678901
    // 99999.123456789
    // 999999.123456789
    // 9999999.12345679
    // [...]
    // 999999999999.123
    // 9999999999999.12
    // 99999999999999.1
    // 999999999999999
    // 1e+16
    // 1e+17
    //
    // For 1*.111111111111111111111111111111111111:
    // 1111111111111.11
    // 11111111111111.1
    // 111111111111111
    // 1.11111111111111e+15
    // 1.11111111111111e+16
    //
    // For 1.234567890123456789 * 10^-i:
    // 1.23456789012346
    // 0.123456789012346
    // 0.0123456789012346
    // 0.00123456789012346
    // 0.000123456789012346
    // 0.0000123456789012346
    // 0.00000123456789012346
    // 1.23456789012346e-6
    // 1.23456789012346e-7

	// Handle non-numeric values.
	if (isNaN(val)) return "NaN";
	
    if (isInf(val)) return val < 0 ? "-Infinity" : "Infinity";

    if (val == 0.0 || val == -0.0) return "0"; 

    std::ostringstream ostr;

	if (radix == 10) {

		// ActionScript always expects dot as decimal point.
		ostr.imbue(std::locale::classic()); 
		
		// force to decimal notation for this range (because the
        // reference player does)
		if (std::abs(val) < 0.0001 && std::abs(val) >= 0.00001) {

			// All nineteen digits (4 zeros + up to 15 significant digits)
			ostr << std::fixed << std::setprecision(19) << val;
			
            std::string str = ostr.str();
			
			// Because 'fixed' also adds trailing zeros, remove them.
			std::string::size_type pos = str.find_last_not_of('0');
			if (pos != std::string::npos) {
				str.erase(pos + 1);
			}
            return str;
		}

        ostr << std::setprecision(15) << val;
        
        std::string str = ostr.str();
        
        // Remove a leading zero from 2-digit exponent if any
        std::string::size_type pos = str.find("e", 0);

        if (pos != std::string::npos && str.at(pos + 2) == '0') {
            str.erase(pos + 2, 1);
        }

        return str;
	}

    // Radix isn't 10
	bool negative = (val < 0);
	if (negative) val = -val;

	double left = std::floor(val);
	if (left < 1) return "0";

    std::string str;
    const std::string digits = "0123456789abcdefghijklmnopqrstuvwxyz";

    // Construct the string backwards for speed, then reverse.
    while (left) {
		double n = left;
		left = std::floor(left / radix);
		n -= (left * radix);
		str.push_back(digits[static_cast<int>(n)]);
	}
	if (negative) str.push_back('-'); 

    std::reverse(str.begin(), str.end());

	return str;
}

/// Force type to number.
as_value&
convertToNumber(as_value& v, VM& /*vm*/)
{
    v.set_double(v.to_number());
    return v;
}

/// Force type to string.
as_value&
convertToString(as_value& v, VM& vm)
{
    v.set_string(v.to_string(vm.getSWFVersion()));
    return v;
}

/// Force type to bool.
as_value&
convertToBoolean(as_value& v, VM& /*vm*/)
{
    v.set_bool(v.to_bool());
    return v;
}

as_value&
convertToPrimitive(as_value& v, VM& vm)
{
    const as_value::AsType t(v.defaultPrimitive(vm.getSWFVersion()));
    v = v.to_primitive(t);
    return v;
}

#if 0

/// Instantiate this value from an AMF element 
as_value::as_value(const amf::Element& el)
	:
	_type(UNDEFINED)
{
    
    VM& vm = VM::get();
    string_table& st = vm.getStringTable();
    Global_as& gl = *vm.getGlobal();

    switch (el.getType()) {
      case amf::Element::NOTYPE:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
	  log_debug("as_value(Element&) : AMF type NO TYPE!");
#endif
	  break;
      }
      case amf::Element::NULL_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
            log_debug("as_value(Element&) : AMF type NULL");
#endif
            set_null();
            break;
      }
      case amf::Element::UNDEFINED_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
            log_debug("as_value(Element&) : AMF type UNDEFINED");
#endif
            set_undefined();
            break;
      }
      case amf::Element::MOVIECLIP_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
            log_debug("as_value(Element&) : AMF type DISPLAYOBJECT");
#endif
            log_unimpl("DISPLAYOBJECT AMF0 type");
            set_undefined();
            //_type = DISPLAYOBJECT;
            //_value = el.getData();

            break;
      }
      case amf::Element::NUMBER_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
            log_debug("as_value(Element&) : AMF type NUMBER");
#endif
            double num = el.to_number();
            set_double(num);
            break;
      }
      case amf::Element::BOOLEAN_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
            log_debug("as_value(Element&) : AMF type BOOLEAN");
#endif
            bool flag = el.to_bool();
            set_bool(flag);
            break;
      }

      case amf::Element::STRING_AMF0:
      case amf::Element::LONG_STRING_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
            log_debug("as_value(Element&) : AMF type STRING");
#endif
	    std::string str;
	    // If there is data, convert it to a string for the as_value
	    if (el.getDataSize() != 0) {
		str = el.to_string();
		// Element's store the property name as the name, not as data.
	    } else if (el.getNameSize() != 0) {
		str = el.getName();
	    }
	    
	    set_string(str);
            break;
      }

      case amf::Element::OBJECT_AMF0:
      {

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
          log_debug("as_value(Element&) : AMF type OBJECT");
#endif
          as_object* obj = gl.createObject();
          if (el.propertySize()) {
              for (size_t i=0; i < el.propertySize(); i++) {
		  const boost::shared_ptr<amf::Element> prop = el.getProperty(i);
		  if (prop == 0) {
		      break;
		  } else {
		      if (prop->getNameSize() == 0) {
			  log_debug("%s:(%d) Property has no name!", __PRETTY_FUNCTION__, __LINE__);
		      } else {
			  obj->set_member(st.find(prop->getName()), as_value(*prop));
		      }
		  }
              }
          }
	  set_as_object(obj);
          break;
      }

      case amf::Element::ECMA_ARRAY_AMF0:
      {
          // TODO: fixme: ECMA_ARRAY has an additional fiedl, dunno
          //              if accessible trought Element class
          //              (the theoretic number of elements in it)

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
          log_debug("as_value(Element&) : AMF type ECMA_ARRAY");
#endif

          as_object* obj = gl.createArray();

          if (el.propertySize()) {
              for (size_t i=0; i < el.propertySize(); i++) {
		  const boost::shared_ptr<amf::Element> prop = el.getProperty(i);
		  if (prop == 0) {
		      break;
		  } else {
		      obj->set_member(st.find(prop->getName()), as_value(*prop));
		  }
              }
          }
          set_as_object(obj);
          break;
      }
    

      case amf::Element::STRICT_ARRAY_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
          log_debug("as_value(Element&) : AMF type STRICT_ARRAY");
#endif
          as_object* obj = gl.createArray();
          size_t len = el.propertySize();
          obj->set_member(NSV::PROP_LENGTH, len);

          for (size_t i=0; i < el.propertySize(); i++) {
              const boost::shared_ptr<amf::Element> prop = el.getProperty(i);
              if (prop == 0) {
                  break;
              } else {
		  if (prop->getNameSize() == 0) {
		      log_debug("%s:(%d) Property has no name!", __PRETTY_FUNCTION__, __LINE__);
		  } else {
		      obj->set_member(st.find(prop->getName()), as_value(*prop));
		  }
              }
          }
          
          set_as_object(obj);
          break;
      }

      case amf::Element::REFERENCE_AMF0:
      {
        log_unimpl("REFERENCE Element to as_value");
        break;
      }

      case amf::Element::DATE_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
	  log_debug("as_value(Element&) : AMF type DATE");
#endif
	  double num = el.to_number();
	  set_double(num);
	  break;
      }
      //if (swfVersion > 5) _type = STRING;
      
      case amf::Element::UNSUPPORTED_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
	  log_debug("as_value(Element&) : AMF type UNSUPPORTED");
#endif
	  break;
      }
      case amf::Element::RECORD_SET_AMF0:
          log_unimpl("Record Set data type is not supported yet");
          break;
      case amf::Element::XML_OBJECT_AMF0:
          log_unimpl("XML data type is not supported yet");
          break;
      case amf::Element::TYPED_OBJECT_AMF0:
          log_unimpl("Typed Object data type is not supported yet");
          break;
      case amf::Element::AMF3_DATA:
          log_unimpl("AMF3 data type is not supported yet");
          break;
      default:
          log_unimpl("Element to as_value - unsupported Element type %d", 
                  el.getType());
          break;
    }
}
#endif
} // namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
