/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2009-2013  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef SCRIPTING_FLASH_EVENTS_FLASHEVENTS_H
#define SCRIPTING_FLASH_EVENTS_FLASHEVENTS_H 1

#include "compat.h"
#include "asobject.h"
#include "backends/extscriptobject.h"
#include <string>
#include <SDL2/SDL_keyboard.h>
#undef MOUSE_EVENT

namespace lightspark
{

enum EVENT_TYPE { EVENT=0, BIND_CLASS, SHUTDOWN, SYNC, MOUSE_EVENT,
	FUNCTION, EXTERNAL_CALL, CONTEXT_INIT, INIT_FRAME,
	FLUSH_INVALIDATION_QUEUE, ADVANCE_FRAME, PARSE_RPC_MESSAGE,EXECUTE_FRAMESCRIPT };

class ABCContext;
class DictionaryTag;
class InteractiveObject;
class PlaceObject2Tag;
class DisplayObject;
class Responder;

class Event: public ASObject
{
public:
	Event(Class_base* cb, const tiny_string& t = "Event", bool b=false, bool c=false, CLASS_SUBTYPE st=SUBTYPE_EVENT);
	void finalize();
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o);
	virtual void setTarget(asAtom t) {target = t; }
	ASFUNCTION_ATOM(_constructor);
	ASFUNCTION_ATOM(_preventDefault);
	ASFUNCTION_ATOM(_isDefaultPrevented);
	ASFUNCTION_ATOM(formatToString);
	ASFUNCTION_ATOM(clone);
	virtual EVENT_TYPE getEventType() const {return EVENT;}
	ASPROPERTY_GETTER(bool,bubbles);
	ASPROPERTY_GETTER(bool,cancelable);
	bool defaultPrevented;
	ASPROPERTY_GETTER(uint32_t,eventPhase);
	ASPROPERTY_GETTER(tiny_string,type);
	//Altough events may be recycled and sent to more than a handler, the target property is set before sending
	//and the handling is serialized
	ASPROPERTY_GETTER(asAtom,target);
	ASPROPERTY_GETTER(_NR<ASObject>,currentTarget);
	ASFUNCTION_ATOM(stopPropagation);
	ASFUNCTION_ATOM(stopImmediatePropagation);
private:
	/*
	 * To be implemented by each derived class to allow redispatching
	 */
	virtual Event* cloneImpl() const;
};

/* Base class for all events that the one can wait on */
class WaitableEvent : public Event
{
private:
	bool handled;
public:
	WaitableEvent(const tiny_string& t = "Event", bool b=false, bool c=false)
		: Event(NULL,t,b,c,SUBTYPE_WAITABLE_EVENT), handled(false) {}
	void wait();
	void signal();
	void finalize();
};

class EventPhase: public ASObject
{
public:
	EventPhase(Class_base* c):ASObject(c){}
	enum
	{
		CAPTURING_PHASE = 1,
		AT_TARGET = 2,
		BUBBLING_PHASE = 3
	};
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o) {}
};

class KeyboardEvent: public Event
{
private:
	virtual Event* cloneImpl() const;

	uint32_t modifiers;
	ASPROPERTY_GETTER_SETTER(uint32_t, charCode);
	ASPROPERTY_GETTER_SETTER(uint32_t, keyCode);
	ASPROPERTY_GETTER_SETTER(uint32_t, keyLocation);
public:
	KeyboardEvent(Class_base* c, tiny_string type="", uint32_t charcode=0, uint32_t keycode=0, SDL_Keymod modifiers=KMOD_NONE);
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o)
	{
	}
	ASFUNCTION_ATOM(_constructor);
	ASFUNCTION_GETTER_SETTER(altKey);
	ASFUNCTION_GETTER_SETTER(commandKey);
	ASFUNCTION_GETTER_SETTER(controlKey);
	ASFUNCTION_GETTER_SETTER(ctrlKey);
	ASFUNCTION_GETTER_SETTER(shiftKey);
};

class FocusEvent: public Event
{
public:
	FocusEvent(Class_base* c);
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o)
	{
	}
	ASFUNCTION_ATOM(_constructor);
};

class FullScreenEvent: public Event
{
public:
	FullScreenEvent(Class_base* c);
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o)
	{
	}
	ASFUNCTION_ATOM(_constructor);
};

class NetStatusEvent: public Event
{
private:
	virtual Event* cloneImpl() const;
public:
	NetStatusEvent(Class_base* cb, const tiny_string& l="", const tiny_string& c="");
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o)
	{
	}
	ASFUNCTION_ATOM(_constructor);
};

class HTTPStatusEvent: public Event
{
public:
	HTTPStatusEvent(Class_base* c):Event(c){}
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o)
	{
	}
	ASFUNCTION_ATOM(_constructor);
};

class TextEvent: public Event
{
protected:
	ASPROPERTY_GETTER_SETTER(tiny_string,text);
public:
	TextEvent(Class_base* c, const tiny_string& t = "textEvent");
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o)
	{
	}
	ASFUNCTION_ATOM(_constructor);
};

class ErrorEvent: public TextEvent
{
protected:
	std::string errorMsg;
	Event* cloneImpl() const;
	ASPROPERTY_GETTER(int32_t,errorID);
public:
	ErrorEvent(Class_base* c, const tiny_string& t = "error", const std::string& e = "", int id = 0);
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o)
	{
	}
	ASFUNCTION_ATOM(_constructor);
};

class IOErrorEvent: public ErrorEvent
{
private:
	Event* cloneImpl() const;
public:
	IOErrorEvent(Class_base* c, const tiny_string& t = "ioError", const std::string& e = "", int id = 0);
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o)
	{
	}
};

class SecurityErrorEvent: public ErrorEvent
{
public:
	SecurityErrorEvent(Class_base* c, const std::string& e = "");
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o)
	{
	}
};

class AsyncErrorEvent: public ErrorEvent
{
public:
	AsyncErrorEvent(Class_base* c);
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o)
	{
	}
	ASFUNCTION_ATOM(_constructor);
};

class UncaughtErrorEvent: public ErrorEvent
{
public:
	UncaughtErrorEvent(Class_base* c);
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o)
	{
	}
	ASFUNCTION_ATOM(_constructor);
};


class ProgressEvent: public Event
{
private:
	ASPROPERTY_GETTER_SETTER(number_t,bytesLoaded);
	ASPROPERTY_GETTER_SETTER(number_t,bytesTotal);
	Event* cloneImpl() const;
public:
	ProgressEvent(Class_base* c);
	ProgressEvent(Class_base* c, uint32_t loaded, uint32_t total, const tiny_string& t="progress");
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o);
	ASFUNCTION_ATOM(_constructor);
};

class TimerEvent: public Event
{
public:
	TimerEvent(Class_base* c):Event(c, "DEPRECATED"){}
	TimerEvent(Class_base* c,const tiny_string& t):Event(c,t,true){}
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o)
	{
	}
	ASFUNCTION_ATOM(updateAfterEvent);
};

class MouseEvent: public Event
{
private:
	uint32_t modifiers; 
	Event* cloneImpl() const;
public:
	MouseEvent(Class_base* c);
	MouseEvent(Class_base* c, const tiny_string& t, number_t lx, number_t ly,
		   bool b=true, SDL_Keymod _modifiers=KMOD_NONE,bool _buttonDown = false,
		   _NR<InteractiveObject> relObj = NullRef, int32_t delta=1);
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o);
	void setTarget(asAtom t);
	EVENT_TYPE getEventType() const { return MOUSE_EVENT;}
	ASFUNCTION_ATOM(_constructor);
	ASPROPERTY_GETTER_SETTER(bool,buttonDown);
	ASFUNCTION_GETTER_SETTER(altKey);
	ASFUNCTION_GETTER_SETTER(controlKey);
	ASFUNCTION_GETTER_SETTER(ctrlKey);
	ASPROPERTY_GETTER_SETTER(int32_t,delta);
	ASPROPERTY_GETTER_SETTER(number_t,localX);
	ASPROPERTY_GETTER_SETTER(number_t,localY);
	ASFUNCTION_GETTER_SETTER(shiftKey);
	ASPROPERTY_GETTER(number_t,stageX);
	ASPROPERTY_GETTER(number_t,stageY);
	ASPROPERTY_GETTER_SETTER(_NR<InteractiveObject>,relatedObject);
	ASFUNCTION_ATOM(updateAfterEvent);
};

class NativeDragEvent: public MouseEvent
{
public:
	NativeDragEvent(Class_base* c);
	
	static void sinit(Class_base*);
	ASFUNCTION_ATOM(_constructor);
};

class InvokeEvent: public Event
{
public:
	InvokeEvent(Class_base* c) : Event(c, "invoke"){}
	static void sinit(Class_base* c);
	static void buildTraits(ASObject* o){}
	ASFUNCTION_ATOM(_constructor);
};

class listener
{
friend class EventDispatcher;
private:
	asAtom f;
	int32_t priority;
	/* true: get events in the capture phase
	 * false: get events in the bubble phase
	 */
	bool use_capture;
public:
	explicit listener(asAtom _f, int32_t _p, bool _c)
		:f(_f),priority(_p),use_capture(_c){}
	bool operator==(std::pair<asAtom,bool> r)
	{
		/* One can register the same handle for the same event with
		 * different values of use_capture
		 */
		return (use_capture == r.second) && f.isEqual(getSys(),r.first);
	}
	bool operator<(const listener& r) const
	{
		//The higher the priority the earlier this must be executed
		return priority>r.priority;
	}
};

class IEventDispatcher
{
public:
	static void linkTraits(Class_base* c);
};

class EventDispatcher: public ASObject, public IEventDispatcher
{
private:
	Mutex handlersMutex;
	std::map<tiny_string,std::list<listener> > handlers;
	/*
	 * This will be used when a target is passed to EventDispatcher constructor
	 */
	asAtom forcedTarget;
protected:
	virtual void eventListenerAdded(const tiny_string& eventName) {}
public:
	EventDispatcher(Class_base* c);
	void finalize();
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o);
	void handleEvent(_R<Event> e);
	void dumpHandlers();
	bool hasEventListener(const tiny_string& eventName);
	virtual void defaultEventBehavior(_R<Event> e) {}
	ASFUNCTION_ATOM(_constructor);
	ASFUNCTION_ATOM(addEventListener);
	ASFUNCTION_ATOM(removeEventListener);
	ASFUNCTION_ATOM(dispatchEvent);
	ASFUNCTION_ATOM(_hasEventListener);
};

class StatusEvent: public Event
{
public:
	StatusEvent(Class_base* c) : Event(c, "StatusEvent") {}
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o) {}
};

class DataEvent: public TextEvent
{
private:
	Event* cloneImpl() const;
public:
	DataEvent(Class_base* c, const tiny_string& _data="") : TextEvent(c, "data"), data(_data) {}
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o) {}
	ASFUNCTION_ATOM(_constructor);
	ASPROPERTY_GETTER_SETTER(tiny_string, data);
};

class RootMovieClip;
//Internal events from now on, used to pass data to the execution thread
class BindClassEvent: public Event
{
friend class ABCVm;
private:
	_NR<RootMovieClip> base;
	DictionaryTag* tag;
	tiny_string class_name;
public:
	BindClassEvent(_R<RootMovieClip> b, const tiny_string& c);
	BindClassEvent(DictionaryTag* t, const tiny_string& c);
	static void sinit(Class_base*);
	EVENT_TYPE getEventType() const { return BIND_CLASS;}
};

class ShutdownEvent: public Event
{
public:
	ShutdownEvent() DLL_PUBLIC;
	static void sinit(Class_base*);
	EVENT_TYPE getEventType() const { return SHUTDOWN; }
};


class FunctionEvent: public WaitableEvent
{
friend class ABCVm;
private:
	asAtom f;
	asAtom obj;
	asAtom* args;
	unsigned int numArgs;
public:
	FunctionEvent(asAtom _f, asAtom _obj=asAtom::invalidAtom, asAtom* _args=NULL, uint32_t _numArgs=0);
	~FunctionEvent();
	static void sinit(Class_base*);
	EVENT_TYPE getEventType() const { return FUNCTION; }
};

class ExternalCallEvent: public WaitableEvent
{
friend class ABCVm;
private:
	asAtom f;
	ASObject* const* args;
	ASObject** result;
	bool* thrown;
	tiny_string* exception;
	unsigned int numArgs;
public:
	ExternalCallEvent(asAtom _f, ASObject* const* _args, uint32_t _numArgs,
			  ASObject** _result, bool* _thrown, tiny_string* _exception);
	~ExternalCallEvent();
	static void sinit(Class_base*);
	EVENT_TYPE getEventType() const { return EXTERNAL_CALL; }
};

class ABCContextInitEvent: public Event
{
friend class ABCVm;
private:
	ABCContext* context;
	bool lazy;
public:
	ABCContextInitEvent(ABCContext* c, bool lazy) DLL_PUBLIC;
	static void sinit(Class_base*);
	EVENT_TYPE getEventType() const { return CONTEXT_INIT; }
};

class Frame;

//Event to construct a Frame in the VM context
class InitFrameEvent: public Event
{
friend class ABCVm;
private:
	_NR<DisplayObject> clip;
public:
	InitFrameEvent(_NR<DisplayObject> m) : Event(NULL, "InitFrameEvent"),clip(m) {}
	EVENT_TYPE getEventType() const { return INIT_FRAME; }
};

class ExecuteFrameScriptEvent: public Event
{
friend class ABCVm;
private:
	_NR<DisplayObject> clip;
public:
	ExecuteFrameScriptEvent(_NR<DisplayObject> m):Event(NULL, "ExecuteFrameScriptEvent"),clip(m) {}
	static void sinit(Class_base*);
	EVENT_TYPE getEventType() const { return EXECUTE_FRAMESCRIPT; }
};

class AdvanceFrameEvent: public WaitableEvent
{
public:
	AdvanceFrameEvent(): WaitableEvent("AdvanceFrameEvent") {}
	EVENT_TYPE getEventType() const { return ADVANCE_FRAME; }
};

//Event to flush the invalidation queue
class FlushInvalidationQueueEvent: public Event
{
public:
	FlushInvalidationQueueEvent():Event(NULL, "FlushInvalidationQueueEvent"){}
	EVENT_TYPE getEventType() const { return FLUSH_INVALIDATION_QUEUE; }
};

class ParseRPCMessageEvent: public Event
{
public:
	_NR<ByteArray> message;
	_NR<ASObject> client;
	_NR<Responder> responder;
	ParseRPCMessageEvent(_R<ByteArray> ba, _NR<ASObject> client, _NR<Responder> responder);
	EVENT_TYPE getEventType() const { return PARSE_RPC_MESSAGE; }
	void finalize();
};

class DRMErrorEvent: public ErrorEvent
{
public:
	DRMErrorEvent(Class_base* c);
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o)
	{
	}
	ASFUNCTION_ATOM(_constructor);
};

class DRMStatusEvent: public Event
{
public:
	DRMStatusEvent(Class_base* c);
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o)
	{
	}
	ASFUNCTION_ATOM(_constructor);
};

class VideoEvent: public Event
{
private:
	Event* cloneImpl() const;
public:
	VideoEvent(Class_base* c);
	static void sinit(Class_base*);
	ASFUNCTION_ATOM(_constructor);
	ASPROPERTY_GETTER(tiny_string,status);
};

class StageVideoEvent: public Event
{
private:
	Event* cloneImpl() const;
public:
	StageVideoEvent(Class_base* c);
	static void sinit(Class_base*);
	ASFUNCTION_ATOM(_constructor);
	ASPROPERTY_GETTER(tiny_string,colorSpace);
	ASPROPERTY_GETTER(tiny_string,status);
};

class StageVideoAvailabilityEvent: public Event
{
private:
	Event* cloneImpl() const;
public:
	StageVideoAvailabilityEvent(Class_base* c);
	static void sinit(Class_base*);
	ASFUNCTION_ATOM(_constructor);
	ASPROPERTY_GETTER(tiny_string,availability);
};

class ContextMenuEvent: public Event
{
public:
	ContextMenuEvent(Class_base* c) : Event(c, "ContextMenuEvent") {}
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o) {}
};

class TouchEvent: public Event
{
public:
	TouchEvent(Class_base* c) : Event(c, "TouchEvent") {}
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o) {}
};

class GestureEvent: public Event
{
public:
	GestureEvent(Class_base* c, const tiny_string& t = "GestureEvent") : Event(c, t) {}
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o) {}
};

class PressAndTapGestureEvent: public GestureEvent
{
public:
	PressAndTapGestureEvent(Class_base* c) : GestureEvent(c, "PressAndTapGestureEvent") {}
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o) {}
};
class TransformGestureEvent: public GestureEvent
{
public:
	TransformGestureEvent(Class_base* c) : GestureEvent(c, "TransformGestureEvent") {}
	static void sinit(Class_base*);
	static void buildTraits(ASObject* o) {}
};

class UncaughtErrorEvents: public EventDispatcher
{
public:
	UncaughtErrorEvents(Class_base* c);
	static void sinit(Class_base*);
	ASFUNCTION_ATOM(_constructor);
};

}
#endif /* SCRIPTING_FLASH_EVENTS_FLASHEVENTS_H */
