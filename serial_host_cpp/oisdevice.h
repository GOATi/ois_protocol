﻿#ifndef OIS_DEVICE_INCLUDED
#define OIS_DEVICE_INCLUDED


//------------------------------------------------------------------------------
// The OIS protocol does not set a maximum length on the ASCII names of channels.
// However, the implementation needs to define a fixed sized command buffer for practical reasons.
// A sensible default size is chosen here, but you can override it by defining OIS_MAX_NAME_LENGTH.
#ifndef OIS_MAX_NAME_LENGTH
const static unsigned OIS_MAX_NAME_LENGTH = 120;
#endif

//------------------------------------------------------------------------------
// The internal command buffer size can be overridden by defining OIS_MAX_COMMAND_LENGTH.
#ifndef OIS_MAX_COMMAND_LENGTH //              NIF= 65535, VERY_LONG_ASCII_NAME \0
const static unsigned OIS_MAX_COMMAND_LENGTH = 4    +6     +OIS_MAX_NAME_LENGTH +1;
#endif

//------------------------------------------------------------------------------
// If you have your own logging mechanism, define OIS_INFO to pipe informational messages to your log.
#ifndef OIS_INFO
#define OIS_INFO( fmt, ... ) do{}while(0)
#endif

//------------------------------------------------------------------------------
// If you have your own logging mechanism, define OIS_WARN to pipe behavior warning messages to your log.
#ifndef OIS_WARN
# ifdef _DEBUG
#  include <cstdio>
#  define OIS_WARN( fmt, ... ) printf( fmt, __VA_ARGS__ )
# else
#  define OIS_WARN( fmt, ... ) do{}while(0)
# endif
#endif

//------------------------------------------------------------------------------
// Define OIS_ENABLE_ERROR_LOGGING as 1 to get fully verbose error reporting.
#ifndef OIS_ENABLE_ERROR_LOGGING
# ifdef _DEBUG
#  define OIS_ENABLE_ERROR_LOGGING 1
# else
#  define OIS_ENABLE_ERROR_LOGGING 0
# endif
#endif

//------------------------------------------------------------------------------
// If you want use use your own assert implementation, define OIS_ASSERT as appropriate.
#ifndef OIS_ASSERT
# ifdef _DEBUG
#  include <cassert>
#  define OIS_ASSERT( condition ) assert(condition)
# else
#  define OIS_ASSERT( condition ) do{}while(0)
# endif
#endif

//------------------------------------------------------------------------------
// Like assert, but can hint unreachable code blocks if your compiler supports such hints.
#ifndef OIS_ASSUME
# define OIS_ASSUME( condition ) OIS_ASSERT(condition)
#endif

//------------------------------------------------------------------------------
// If your compiler doesn't supply stdint.h, define OIS_NO_STDINT and include your own implementation.
#ifndef OIS_NO_STDINT
# include <cstdint>
#endif 

//------------------------------------------------------------------------------
// If you want use use your own dynamic array class, define OIS_VECTOR to your own class.
#ifndef OIS_VECTOR
# include <vector>
# define OIS_VECTOR std::vector
#endif

//------------------------------------------------------------------------------
// If you want use use your own string class, define OIS_STRING to your own class.
#ifndef OIS_STRING
# include <string>
# define OIS_STRING std::string
#endif

//------------------------------------------------------------------------------
// If you want use use your own string builder, define OIS_STRING_BUILDER to your own class that implements the interface below.
// FormatTemp, AllocTemp use char-buffers that are owned by the OIS_STRING_BUILDER object:
//   The lifetime of these allocations is as long as this OIS_STRING_BUILDER object, 
//   or until the next "Temp" operation on this OIS_STRING_BUILDER object (whichever is shorter).
// Format, FormatV, StoreTemp use char-buffers that are owned by the first argument.
//------------------------------------------------------------------------------
#ifndef OIS_STRING_BUILDER
# include <stdarg.h>
struct OIS_STRING_BUILDER
{
	const char* FormatTemp(const char* fmt, ...)//Format a string using the "temp" lifetime.
	{
		va_list	v;
		va_start(v, fmt);
		const char* s = FormatV(temp, fmt, v);
		va_end( v );
		return s;
	}
	const char* Format(OIS_STRING& result, const char* fmt, ...)//Format a string using a user-controlled lifetime.
	{
		va_list	v;
		va_start(v, fmt);
		const char* s = FormatV(result, fmt, v);
		va_end( v );
		return s;
	}
	const char* FormatV(OIS_STRING& result, const char* fmt, const va_list& v)//Format a string using a user-controlled lifetime.
	{
		int length = _vscprintf( fmt, v ) + 1;
		result.resize(length);
		char* buffer = &result[0];
		vsnprintf(buffer, length, fmt, v);
		result.resize(length-1);//don't keep the \0 terminator as part of the std::string data
		return buffer;
	}
	char* AllocTemp(unsigned bytes)//allocate N bytes using the "temp" lifetime.
	{
		temp.resize(bytes);
		return &temp[0];
	}
	void StoreTemp(OIS_STRING& result, const char* buffer)//buffer is a value that was returned from AllocTemp
	{
		result = buffer;
	}
private:
	OIS_STRING temp;
};
#endif

//------------------------------------------------------------------------------
// If you want to use your own communication method, define OIS_PORT to your own class.
// If not defined, we will include the bundled SerialPort class.
//------------------------------------------------------------------------------
#ifndef OIS_PORT
# include "serialport.hpp"
# ifdef OIS_VIRTUAL_PORT
#  define OIS_PORT IOisPort
# else
#  define OIS_PORT SerialPort
# endif
#endif



//------------------------------------------------------------------------------
// If you want to use multiple different communication methods, defining OIS_VIRTUAL_PORT will enable dynamic dispatch for communication functions.
// If the bundled SerialPort class was included, the OisPortSerial class is an adaptor to make SerialPort implement the IOisPort interface. 
#ifdef OIS_VIRTUAL_PORT
class IOisPort
{
public:
	virtual ~IOisPort() {}
	virtual bool IsConnected() = 0;
	virtual void Connect() = 0;
	virtual void Disconnect() = 0;
	virtual int  Read(char* buffer, int size) = 0;
	virtual bool Write(const char* buffer, int size) = 0;
	virtual const char* Name() { return ""; }
};

# ifdef OIS_SERIALPORT_INCLUDED
class OisPortSerial : public IOisPort
{
public:
	OisPortSerial(const char* portName)
		: m_port(portName)
	{}
	bool IsConnected()                       { return m_port.IsConnected(); }
	void Connect()                           { return m_port.Connect(); }
	void Disconnect()                        { return m_port.Disconnect(); }
	int Read(char* buffer, int size)         { return m_port.Read(buffer, size); }
	bool Write(const char* buffer, int size) { return m_port.Write(buffer, size); }
	virtual const char* Name()               { return m_port.PortName().c_str(); }
private:
	SerialPort m_port;
};
# endif
#endif


//------------------------------------------------------------------------------
// Generic utilities: Length of fixed C arrays
#ifndef OIS_ARRAYSIZE
namespace OisDeviceInternal
{
	template<unsigned N> struct Sizer { char elems[N]; };
	template<class Type, unsigned N> Sizer<N> ArraySize_(Type(&)[N]);
}
# define OIS_ARRAYSIZE( a ) sizeof( OisDeviceInternal::ArraySize_( a ).elems )
#endif

//------------------------------------------------------------------------------
// Generic utilities: uint32_t FOURCC code from a 4-character string
#ifndef OIS_FOURCC
# define OIS_FOURCC(str)                                                         \
	((uint32_t)(uint8_t)(str[0])        | ((uint32_t)(uint8_t)(str[1]) << 8) |   \
	((uint32_t)(uint8_t)(str[2]) << 16) | ((uint32_t)(uint8_t)(str[3]) << 24 ))  //
#endif


//------------------------------------------------------------------------------
// Shared structures / utilities between host and device
class OisState
{
public:
	enum NumericType
	{
		Boolean,
		Number,
		Fraction,
	};
	typedef union {
		bool    boolean;
		int32_t number;
		float   fraction;
	} Value;
	struct NumericValue
	{
		OIS_STRING  name;
		uint16_t    channel;
		bool        active;
		NumericType type;
		Value       value;
	};
	struct Event
	{
		uint16_t channel;
		OIS_STRING name;
	};
protected:
	enum ClientCommandsAscii
	{
		//v1 commands
		SYN = OIS_FOURCC("SYN="),
		CMD = OIS_FOURCC("CMD="),
		NIB = OIS_FOURCC("NIB="),
		NIN = OIS_FOURCC("NIN="),
		NIF = OIS_FOURCC("NIF="),
		ACT = OIS_FOURCC("ACT\0"),
		EXC = OIS_FOURCC("EXC="),
		DBG = OIS_FOURCC("DBG="),
		//v2 commands
		NOB = OIS_FOURCC("NOB="),
		NON = OIS_FOURCC("NON="),
		NOF = OIS_FOURCC("NOF="),
		TNI = OIS_FOURCC("TNI="),
		PID = OIS_FOURCC("PID="),
		END = OIS_FOURCC("END\0"),
	};
	enum ClientCommandsBin
	{
		CL_NUL = 0x00,
		CL_CMD = 0x01,
		CL_NIO = 0x02,
		CL_ACT = 0x03,
		CL_SYN_ = 'S',//0x53
		CL_DBG = 0x04,
		CL_451_ = '4',//0x34
		CL_END_ = 'E',//0x45
		CL_TNI = 0x05,
		CL_PID = 0x06,
		CL_END = 0x07,
		CL_VAL_1 = 0x08,
		CL_VAL_2 = 0x09,
		CL_VAL_3 = 0x0A,
		CL_VAL_4 = 0x0B,
		CL_EXC_0 = 0x0C,
		CL_EXC_1 = 0x0D,
		CL_EXC_2 = 0x0E,

		CL_COMMAND_MASK = 0x0F,
		CL_PAYLOAD_SHIFT = 4,

		CL_N_PAYLOAD_N = 0x10,
		CL_N_PAYLOAD_F = 0x20,
		CL_N_PAYLOAD_O = 0x40,

		CL_TNI_PAYLOAD_T = 0x10,
	};
	enum ServerCommandsBin
	{
		SV_NUL = 0x00,
		SV_VAL_1 = 0x01,
		SV_VAL_2 = 0x02,
		SV_VAL_3 = 0x03,
		SV_VAL_4 = 0x04,
		SV_END_ = 'E',//0x45

		SV_COMMAND_MASK = 0x07,
		SV_PAYLOAD_SHIFT = 3,
	};
	enum DeviceState
	{
		Handshaking,
		Synchronisation,
		Active,
	};
	typedef uint32_t DeviceStateMask;

	static uint16_t ToRawValue(NumericType type, Value value);
	static Value    FromRawValue(NumericType type, uint16_t value);
	static int      PackNumericValueCommand(const NumericValue& v, uint8_t cmd[5], unsigned PAYLOAD_SHIFT, unsigned VAL_1, unsigned VAL_2, unsigned VAL_3, unsigned VAL_4);
	static bool     SetValueAndEnqueue(const NumericValue& input, Value value, OIS_VECTOR<NumericValue>& values, OIS_VECTOR<uint16_t> queue);
	static int      CmdStrLength(const char* c, const char* end, char terminator);
	static char*    ZeroDelimiter(char* str, char delimiter);
	template<class T> static typename T::value_type* FindChannel(T& values, int channel);
	template<class T> static T Clamp(T x, T min, T max) { return x < min ? min : (x > max ? max : x); }
};


//------------------------------------------------------------------------------
// Shared internal data / logic between device and host. Uses compile-time polymorphism.
template<class CRTP> class OisBase : protected OisState
{
protected:
	bool ReadCommands();
	void ProcessCommands();
	void SendData(const char* cmd, int length);
	void SendText(const char* cmd);
	void SendValue(const NumericValue& v, OIS_STRING_BUILDER& sb, unsigned PAYLOAD_SHIFT, unsigned VAL_1, unsigned VAL_2, unsigned VAL_3, unsigned VAL_4);
	void ConnectAndPoll();
	bool ExpectState(DeviceStateMask state, const char* cmd, unsigned version);
	void _ClearState()                                    { return static_cast<CRTP*>(this)->ClearState(); }
	bool _ProcessAscii(char* cmd, OIS_STRING_BUILDER& sb) { return static_cast<CRTP*>(this)->ProcessAscii(cmd, sb); }
	int  _ProcessBinary(char* start, char* end)           { return static_cast<CRTP*>(this)->ProcessBinary(start, end); }

	OIS_PORT& m_port;
	OIS_STRING m_deviceName;

	unsigned m_gameVersion = 0;
	OIS_STRING m_gameName;

	unsigned m_protocolVersion = 1;
	bool     m_binary = false;
	uint32_t m_pid = 0;
	uint32_t m_vid = 0;
	DeviceState m_connectionState = Handshaking;
	char m_commandBuffer[OIS_MAX_COMMAND_LENGTH * 2];
	unsigned m_commandLength = 0;

	OIS_VECTOR<NumericValue> m_numericInputs;
	OIS_VECTOR<NumericValue> m_numericOutputs;
	OIS_VECTOR<Event>        m_events;

	OisBase(OIS_PORT& port, const OIS_STRING& name, unsigned gameVersion, const char* gameName)
		: m_port(port)
		, m_deviceName(name)
		, m_gameVersion(gameVersion)
		, m_gameName(gameName)
	{
		_ClearState();
	}

	OisBase(OIS_PORT& port, const OIS_STRING& name, uint32_t pid, uint32_t vid)
		: m_port(port)
		, m_deviceName(name)
		, m_pid(pid)
		, m_vid(vid)
	{
		_ClearState();
	}
};

//------------------------------------------------------------------------------
//Use this class on the host to talk to a device
class OisDevice : private OisBase<OisDevice>
{
public:
	OisDevice(OIS_PORT& port, const OIS_STRING& name, unsigned gameVersion, const char* gameName)
		: OisBase(port, name, gameVersion, gameName)
	{}
	const char* GetDeviceName() const { return m_deviceNameOverride.empty() ? m_deviceName.c_str() : m_deviceNameOverride.c_str(); }
	uint32_t    GetProductID()  const { return m_pid; }
	uint32_t    GetVendorID()   const { return m_vid; }
	bool        Connecting()    const { return m_connectionState != Handshaking; }
	bool        Connected()     const { return m_connectionState == Active; }
	
	const OIS_VECTOR<NumericValue>& DeviceInputs()  const { return m_numericInputs; }
	const OIS_VECTOR<NumericValue>& DeviceOutputs() const { return m_numericOutputs; }
	const OIS_VECTOR<Event>&        DeviceEvents()  const { return m_events; }

	void Poll(OIS_STRING_BUILDER&);

	template<class T>
	bool PopEvents(T& fn);//calls fn(const Event&)
	bool SetInput(const NumericValue& input, Value value);
private:
	friend class OisBase<OisDevice>;
	void ClearState();
	bool ProcessAscii(char* cmd, OIS_STRING_BUILDER&);
	int  ProcessBinary(char* start, char* end);

	OIS_STRING           m_deviceNameOverride;
	OIS_VECTOR<uint16_t> m_queuedInputs;
	OIS_VECTOR<uint16_t> m_eventBuffer;
};

//------------------------------------------------------------------------------
//Use this class on the device to talk to a host
class OisHost : private OisBase<OisHost>
{
public:
	OisHost(OIS_PORT& port, const OIS_STRING& name, uint32_t pid, uint32_t vid)
		: OisBase(port, name, pid, vid)
	{
	}

	const OIS_STRING& GetGameName()    const { return m_gameName; }
	unsigned          GetGameVersion() const { return m_gameVersion; }
	bool              Connecting()     const { return m_connectionState != Handshaking; }
	bool              Connected()      const { return m_connectionState == Active; }

	const OIS_VECTOR<NumericValue>& DeviceInputs()  const { return m_numericInputs; }
	const OIS_VECTOR<NumericValue>& DeviceOutputs() const { return m_numericOutputs; }
	const OIS_VECTOR<Event>&        DeviceEvents()  const { return m_events; }

	void Poll(OIS_STRING_BUILDER&);

	void Activate(const Event&);
	bool SetOutput(const NumericValue& output, Value value);
private:
	friend class OisBase<OisHost>;
	void ClearState();
	bool ProcessAscii(char* cmd, OIS_STRING_BUILDER&);
	int  ProcessBinary(char* start, char* end);

	OIS_VECTOR<uint16_t> m_queuedOutputs;
	OIS_VECTOR<uint16_t> m_eventBuffer;
};


//------------------------------------------------------------------------------

template<class T> typename T::value_type* OisState::FindChannel(T& values, int channel)
{
	for (auto& v : values)
		if (v.channel == channel)
			return &v;
	return nullptr;
}

//------------------------------------------------------------------------------

template<class T>
void OisBase<T>::SendData(const char* cmd, int length)
{
	m_port.Write(cmd, length);
}

template<class T>
void OisBase<T>::SendText(const char* cmd)
{
	m_port.Write(cmd, (int)strlen(cmd));
}

template<class T>
void OisBase<T>::SendValue(const NumericValue& v, OIS_STRING_BUILDER& sb, unsigned PAYLOAD_SHIFT, unsigned VAL_1, unsigned VAL_2, unsigned VAL_3, unsigned VAL_4)
{
	if (m_binary)
	{
		uint8_t cmd[5];
		int cmdLength = PackNumericValueCommand(v, cmd, PAYLOAD_SHIFT, VAL_1, VAL_2, VAL_3, VAL_4);
		SendData((char*)cmd, cmdLength);
	}
	else
	{
		int16_t data = ToRawValue(v.type, v.value);
		SendText(sb.FormatTemp("%d=%d\n", v.channel, data));
	}
}

template<class T>
bool OisBase<T>::ReadCommands()
{
	int len = m_port.Read(m_commandBuffer + m_commandLength, OIS_ARRAYSIZE(m_commandBuffer) - m_commandLength);
	if (!len)
		return false;
	m_commandLength += len;
	OIS_ASSERT(m_commandLength <= OIS_ARRAYSIZE(m_commandBuffer));
	return true;
}

template<class T>
void OisBase<T>::ProcessCommands()
{
	char* start = m_commandBuffer;
	char* end = start + m_commandLength;
	if (m_binary)
	{
		while (start < end)
		{
			int commandLength = _ProcessBinary(start, end);
			if (commandLength == 0)//need to read more data
				break;
			if (commandLength < 0)//not a binary command!
			{
				_ClearState();
				return;
			}
			start += commandLength;//processed
		}
	}
	else
	{
		for (char* c = start; c != end; ++c)
		{
			if (*c != '\n')
				continue;
			*c = '\0';
			_ProcessAscii(start, sb);
			start = c + 1;
		}
	}
	OIS_ASSERT(start <= end);

	if (start == m_commandBuffer && end == m_commandBuffer + OIS_ARRAYSIZE(m_commandBuffer))
	{
		OIS_WARN("OisDevice command buffer is full without a valid command present! Ending...");
		OIS_INFO("-> END");
		SendText("END\n");
		_ClearState();
	}
	else
	{
		m_commandLength = (unsigned)(end - start);
		if (start != m_commandBuffer)
			memmove(m_commandBuffer, start, m_commandLength);
	}
}

template<class T>
void OisBase<T>::ConnectAndPoll()
{
	if (!m_port.IsConnected())
	{
		if (m_connectionState != Handshaking)
			_ClearState();
		m_port.Connect();
		return;
	}
	for (;;)
	{
		if (!ReadCommands())
			break;
		ProcessCommands();
	}
}

template<class T>
bool OisBase<T>::ExpectState(DeviceStateMask state, const char* cmd, unsigned version)
{
	if (m_protocolVersion < version)
		OIS_WARN("Did not expect command under version #%d: %s", m_protocolVersion, cmd);
	bool badState = 0 == ((1 << m_connectionState) & state);
	if (badState)
	{
		OIS_WARN("Did not expect command at this time: %s", cmd);
		if (m_connectionState == Handshaking)
		{
			_ClearState();
			SendText("END\n");
		}
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------

template<class T>
bool OisDevice::PopEvents(T& fn)
{
	if (m_eventBuffer.empty())
		return false;
	for (uint16_t channel : m_eventBuffer)
	{
		const Event* e = FindChannel(m_events, channel);
		if (e)
			fn(*e);
	}
	m_eventBuffer.clear();
	return true;
}

//------------------------------------------------------------------------------
#ifdef OIS_DEVICE_IMPL
//------------------------------------------------------------------------------

char* OisState::ZeroDelimiter(char* str, char delimiter)
{
	char* c = str;
	for (; *c; ++c)
	{
		if (*c == delimiter)
		{
			*c = '\0';
			return c + 1;
		}
	}
	return c;
}

int OisState::CmdStrLength(const char* c, const char* end, char terminator)
{
	int length = 0;
	bool foundNull = false;
	for (; c != end && !foundNull; ++c, ++length)
		foundNull = *c == terminator;
	return length + (foundNull ? 0 : OIS_MAX_COMMAND_LENGTH * 2);
}

uint16_t OisState::ToRawValue(NumericType type, Value value)
{
	switch (type)
	{
	default: OIS_ASSERT(false);
	case Boolean:  return value.boolean ? 1 : 0;
	case Number:   return (int16_t)Clamp(value.number, -32768, 32767);
	case Fraction: return (int16_t)Clamp((int)(value.fraction*100.0f), -32768, 32767);
	}
}

OisState::Value OisState::FromRawValue(NumericType type, uint16_t value)
{
	Value v;
	switch (type)
	{
	default: OIS_ASSERT(false);
	case Boolean:  v.boolean = !!value;        break;
	case Number:   v.number = value;          break;
	case Fraction: v.fraction = value / 100.0f; break;
	}
	return v;
}

int OisState::PackNumericValueCommand(const NumericValue& v, uint8_t cmd[5], unsigned PAYLOAD_SHIFT, unsigned VAL_1, unsigned VAL_2, unsigned VAL_3, unsigned VAL_4)
{
	int cmdLength;
	int16_t data = ToRawValue(v.type, v.value);
	uint16_t u = (uint16_t)data;
	const unsigned extraBits = 8 - PAYLOAD_SHIFT;
	const unsigned valueLimit1 = 1U << extraBits;
	const unsigned valueLimit2 = 1U << (8 + extraBits);
	const unsigned channelLimit3 = 1U << (8 + extraBits);
	if (v.channel < 256 && u < valueLimit1)
	{
		cmd[0] = (uint8_t)(0xFF & (VAL_1 | (u << PAYLOAD_SHIFT)));
		cmd[1] = (uint8_t)(0xFF & v.channel);
		cmdLength = 2;
	}
	else if (v.channel < 256 && u < valueLimit2)
	{
		cmd[0] = (uint8_t)(0xFF & (VAL_2 | ((u >> 8) << PAYLOAD_SHIFT)));
		cmd[1] = (uint8_t)(0xFF & u);
		cmd[2] = (uint8_t)(0xFF & v.channel);
		cmdLength = 3;
	}
	else if (v.channel < channelLimit3)
	{
		cmd[0] = (uint8_t)(0xFF & (VAL_3 | ((v.channel >> 8) << PAYLOAD_SHIFT)));
		cmd[1] = (uint8_t)(0xFF & u);
		cmd[2] = (uint8_t)(0xFF & (u >> 8));
		cmd[3] = (uint8_t)(0xFF & v.channel);
		cmdLength = 4;
	}
	else
	{
		cmd[0] = (uint8_t)VAL_4;
		cmd[1] = (uint8_t)(0xFF & u);
		cmd[2] = (uint8_t)(0xFF & (u >> 8));
		cmd[3] = (uint8_t)(0xFF & v.channel);
		cmd[4] = (uint8_t)(0xFF & (v.channel >> 8));
		cmdLength = 5;
	}
	return cmdLength;
}

bool OisState::SetValueAndEnqueue(const NumericValue& input, Value value, OIS_VECTOR<NumericValue>& values, OIS_VECTOR<uint16_t> queue)
{
	if (values.empty())
		return false;
	unsigned index = (unsigned)(&input - &values.front());
	if (index >= values.size())
		return false;
	if (values[index].value.number != value.number)
	{
		values[index].value = value;
		queue.push_back(index);
	}
	return true;
}

//------------------------------------------------------------------------------

void OisDevice::Poll(OIS_STRING_BUILDER& sb)
{
	ConnectAndPoll();
	for (int index : m_queuedInputs)
	{
		const NumericValue& v = m_numericInputs[index];
		switch (v.type)
		{
		case Boolean:  OIS_INFO("-> %d(%s) = %s",   v.channel, v.name.c_str(), v.value.boolean ? "true" : "false"); break;
		case Number:   OIS_INFO("-> %d(%s) = %d",   v.channel, v.name.c_str(), v.value.number);                     break;
		case Fraction: OIS_INFO("-> %d(%s) = %.2f", v.channel, v.name.c_str(), v.value.fraction);                   break;
		}
		SendValue(v, sb, SV_PAYLOAD_SHIFT, SV_VAL_1, SV_VAL_2, SV_VAL_3, SV_VAL_4);
	}
	m_queuedInputs.clear();
}

int OisDevice::ProcessBinary(char* start, char* end)
{
	if (end <= start)
		return 0;

	int bufferLength = (int)(end - start);
	
	const char* startString = 0;
	char strTerminator = '\0';
	uint32_t payload = (*start);
	int command = payload & CL_COMMAND_MASK;
	int cmdLength = 1;
	if (payload == CL_SYN_ || payload == CL_451_)//has the device reset and is sending us ASCII commands?
	{
		cmdLength = 0;
		startString = start;
		strTerminator = '\n';
	}
	else switch (command)
	{
		default:
		case CL_NUL:
			OIS_WARN( "Unknown command: 0x%x", payload);
			break;
		case CL_ACT:
		case CL_END:
		case CL_EXC_0:
			break;
		case CL_CMD:
		case CL_NIO:
			cmdLength += 2;//channel
			startString = start + cmdLength;
			break;
		case CL_DBG:
			startString = start + cmdLength;
			break;
		case CL_PID:
			cmdLength += 8;//pid/vid
			startString = start + cmdLength;
			break;
		case CL_EXC_1:
		case CL_VAL_1:
			cmdLength += 1;
			break;
		case CL_TNI:
		case CL_EXC_2:
		case CL_VAL_2:
			cmdLength +=2;
			break;
		case CL_VAL_3:
			cmdLength += 3;
			break;
		case CL_VAL_4:
			cmdLength += 4;
			break;
	}

	if (startString)
	{
		if (startString >= end)
			return 0;
		cmdLength += CmdStrLength(startString, end, strTerminator);
	}

	if (bufferLength < cmdLength)
		return 0;

	//has the device reset and is sending us ASCII commands?
	if ((payload == CL_SYN_ && 0 == strcmp(startString, "SYN")) ||
		(payload == CL_451_ && 0 == strcmp(startString, "451")))
	{
		return -1;
	}

	switch (command)
	{
		case CL_PID:
		{
			ExpectState( 1<<Synchronisation, "PID", 2 );
			m_pid = *(uint32_t*)(start+1);
			m_vid = *(uint32_t*)(start+5);
			char* name = start+9;
			m_deviceNameOverride = OIS_STRING(name);
			OIS_INFO( "<- PID: %d/%d %s", m_pid, m_vid, name );
			break;
		}
		case CL_CMD:
		{
			if( !ExpectState( (1<<Synchronisation)|(1<<Active), "CMD", 2 ) )
				return false;
			uint16_t channel = *(uint16_t*)(start+1);
			char* name = start+3;
			m_events.push_back({channel, OIS_STRING(name)});
			OIS_INFO( "<- CMD: %d %s", channel, name );
			break;
		}
		case CL_NIO:
		{
			if( !ExpectState( (1<<Synchronisation)|(m_protocolVersion>1?1<<Active:0), "NIO", 2 ) )
				return false;
			bool output = payload & CL_N_PAYLOAD_O ? true :false;
			NumericType nt = payload & CL_N_PAYLOAD_F ? Fraction :
			                (payload & CL_N_PAYLOAD_N ? Number : Boolean);
			uint16_t channel = *(uint16_t*)(start+1);
			char* name = start+3;
			OIS_VECTOR<NumericValue>& vec = output ? m_numericOutputs : m_numericInputs;
			vec.push_back({OIS_STRING(name), channel, true, nt});
			vec.back().value.number = 0;
			OIS_INFO( "<- NIO: %d %s (%s %s)", channel, name, output?"Out":"In", nt==Fraction?"Fraction":(nt==Number?"Number":"Boolean") );
			break;
		}
		case CL_ACT:
		{
			ExpectState( 1<<Synchronisation, "ACT", 2 );
			m_connectionState = Active;
			OIS_INFO( "<- ACT" );
			break;
		}
		case CL_TNI:
		{
			ExpectState( (1<<Synchronisation) | (1<<Active), "TNI", 2 );
			uint16_t channel = *(uint16_t*)(start+1);
			NumericValue* v = FindChannel(m_numericInputs, channel);
			OIS_INFO( "<- TNI %d (%s)", channel, v?v->name.c_str():"UNKNOWN CHANNEL" );
			if( v )
				v->active = (payload & CL_TNI_PAYLOAD_T) ? true: false;
			break;
		}
		case CL_DBG:
		{
			OIS_INFO( "<- DBG: %s", startString);
			break;
		}
		case CL_EXC_0:
		case CL_EXC_1:
		case CL_EXC_2:
		{
			ExpectState( 1<<Active, "EXC", 2 );
			uint16_t channel = 0;
			uint16_t extra = (uint16_t)(payload >> CL_PAYLOAD_SHIFT);
			switch( command )
			{
			default: OIS_ASSUME(false);
			case CL_EXC_0: channel = extra; break;
			case CL_EXC_1: channel = (*(uint8_t *)(start+1)) | (extra<<8); break;
			case CL_EXC_2: channel = (*(uint16_t*)(start+1));              break;
			}
			Event* e = FindChannel(m_events, channel);
			if( e )
				m_eventBuffer.push_back(channel);
			OIS_INFO( "<- EXC: %d (%s)", channel, e ? e->name.c_str() : "INVALID CHANNEL" );
			break;
		}
		case CL_VAL_1:
		case CL_VAL_2:
		case CL_VAL_3:
		case CL_VAL_4:
		{
			if( !ExpectState( 1<<Active, "VAL", 2 ) )
				return false;
			int16_t channel = 0;
			uint32_t extra = payload >> CL_PAYLOAD_SHIFT;
			int16_t value = 0;
			switch( command )
			{
			default: OIS_ASSUME(false);
			case CL_VAL_1: value = extra;                              channel = *(uint8_t *)(start+1);              break;
			case CL_VAL_2: value = *(uint8_t *)(start+1)|(extra << 8); channel = *(uint8_t *)(start+2);              break;
			case CL_VAL_3: value = *(uint16_t*)(start+1);              channel = *(uint8_t *)(start+3)|(extra << 8); break;
			case CL_VAL_4: value = *(uint16_t*)(start+1);              channel = *(uint16_t*)(start+3);              break;
			}
			NumericValue* v = FindChannel(m_numericOutputs, channel);
			if( v )
			{
				v->value = FromRawValue(v->type, value);
				switch( v->type )
				{
				case Boolean:  OIS_INFO( "<- %d(%s) = %s",   channel, v->name.c_str(), v->value.boolean ? "true" : "false" ); break;
				case Number:   OIS_INFO( "<- %d(%s) = %d",   channel, v->name.c_str(), v->value.number );                     break;
				case Fraction: OIS_INFO( "<- %d(%s) = %.2f", channel, v->name.c_str(), v->value.fraction );                   break;
				}
			}
			else
				OIS_WARN( "Received key/value message for unregistered channel %d", channel);
			break;
		}
		case CL_END:
		{
			OIS_INFO( "<- END");
			if( m_connectionState != Handshaking )
				ClearState();
			m_port.Disconnect();
			break;
		}
	}

	return cmdLength;
}

bool OisDevice::ProcessAscii(char* cmd, OIS_STRING_BUILDER& sb)
{
	if (!cmd[0])
		return false;
	uint32_t type = 0;
	if (cmd[1] && cmd[2])
		type = OIS_FOURCC(cmd);
	bool isKeyVal = 0 != isdigit(cmd[0]);
	if (isKeyVal)
	{
		if (!ExpectState(1 << Active, cmd, 2))
			return false;
		const char* payload = ZeroDelimiter(cmd, '=');
		int channel = atoi(cmd);
		NumericValue* v = FindChannel(m_numericOutputs, channel);
		if (v)
		{
			v->value = FromRawValue(v->type, atoi(payload));
			switch (v->type)
			{
			case Boolean:  OIS_INFO("<- %d(%s) = %s",   channel, v->name.c_str(), v->value.boolean ? "true" : "false"); break;
			case Number:   OIS_INFO("<- %d(%s) = %d",   channel, v->name.c_str(), v->value.number);                     break;
			case Fraction: OIS_INFO("<- %d(%s) = %.2f", channel, v->name.c_str(), v->value.fraction);                   break;
			}
		}
		else
			OIS_WARN("Received key/value message for unregistered channel %d", channel);
	}
	else
	{
		char* payload = cmd + 4;
		switch (type)
		{
			default:
			{
				OIS_WARN( "Unknown command: %s", cmd);
				break;
			}
			case SYN:
			{
				if( !ExpectState(1<<Handshaking, cmd, 1) )
					ClearState();
				char* mode = ZeroDelimiter(payload, ',');
				bool binary = *mode == 'B';
				int version = atoi(payload);
				OIS_INFO( "<- SYN: %d/%s", version, binary?"B":"A" );
				if( !(version == 1 && binary) && version >= 1 && version <= 2 )
				{
					m_binary = binary;
					m_protocolVersion = version;
					m_connectionState = Synchronisation;
					switch( version )
					{
					default: OIS_ASSUME(false);
					case 1: SendText("ACK\n");        break;
					case 2: SendText(sb.FormatTemp("ACK=%d,%s\n", m_gameVersion, m_gameName)); break;
					}
					OIS_INFO( "-> ACK", version );
				}
				else
				{
					OIS_INFO( "-> DEN", version );
					SendText("DEN\n");
					ClearState();
				}
				break;
			}
			case PID:
			{
				ExpectState(1<<Synchronisation, cmd, 2);
				char* pid = payload;
				char* vid = ZeroDelimiter(pid, ',');
				char* name = ZeroDelimiter(vid, ',');
				m_pid = atoi(pid);
				m_vid = atoi(vid);
				m_deviceNameOverride = name;
				OIS_INFO( "<- PID: %d/%d %s", m_pid, m_vid, name );
				break;
			}
			case CMD:
			{
				if( !ExpectState( (1<<Synchronisation)|(m_protocolVersion>1?1<<Active:0), cmd, 1 ) )
					return false;
				char* name = payload;
				uint16_t channel = (uint16_t)(0xFFFF & atoi(ZeroDelimiter(payload, ',')));
				m_events.push_back({channel, OIS_STRING(name)});
				OIS_INFO( "<- CMD: %d %s", channel, name );
				break;
			}
			case NIN: case NIF: case NIB:
			case NON: case NOF: case NOB:
			{
				bool output = false;
				NumericType nt;
				switch( type )
				{
				default: OIS_ASSUME( false );
				case NON: output = true;
				case NIN: nt = Number;	 break;
				case NOF: output = true;
				case NIF: nt = Fraction; break;
				case NOB: output = true;
				case NIB: nt = Boolean;	 break;
				}
				if( !ExpectState( (1<<Synchronisation)|(m_protocolVersion>1?1<<Active:0), cmd, output ? 2 : 1 ) )
					return false;
				char* name = payload;
				int channel = atoi(ZeroDelimiter(payload, ','));
				uint16_t channel16 = (uint16_t)(channel & 0xFFFFU);
				OIS_VECTOR<NumericValue>& vec = output ? m_numericOutputs : m_numericInputs;
				vec.push_back({OIS_STRING(name), channel16, true, nt});
				vec.back().value.number = 0;
				OIS_INFO( "<- %s: %d %s", cmd, channel16, name );
				break;
			}
			case TNI:
			{
				ExpectState( (1<<Synchronisation) | (1<<Active), cmd, 2 );
				const char* active = ZeroDelimiter(payload, ',');
				int channel = atoi(payload);
				NumericValue* v = FindChannel(m_numericInputs, channel);
				OIS_INFO( "<- TNI %d (%s)", channel, v?v->name.c_str():"UNKNOWN CHANNEL" );
				if( v )
					v->active = atoi(active) ? true: false;
			}
			case ACT:
			{
				ExpectState( 1<<Synchronisation, cmd, 1 );
				m_connectionState = Active;
				OIS_INFO( "<- ACT" );
				break;
			}

			case EXC:
			{
				ExpectState( 1<<Active, cmd, 1 );
				int channel = atoi(payload);
				Event* e = FindChannel(m_events, channel);
				if( e )
					m_eventBuffer.push_back(channel);
				OIS_INFO( "<- EXC: %d (%s)", channel, e ? e->name.c_str() : "INVALID CHANNEL" );
				break;
			}
			case DBG:
			{
				OIS_INFO( "<- DBG: %s", payload);
				break;
			}
			case END:
			{
				OIS_INFO( "<- END");
				if( m_connectionState != Handshaking )
					ClearState();
				m_port.Disconnect();
				break;
			}
		}
	}
	return true;
}

bool OisDevice::SetInput(const NumericValue& input, Value value)
{
	return SetValueAndEnqueue(input, value, m_numericInputs, m_queuedInputs);
}

void OisDevice::ClearState()
{
	m_connectionState = Handshaking;
	m_protocolVersion = 1;
	m_binary = false;
	m_pid = OIS_FOURCC("NULL");
	m_vid = OIS_FOURCC("OIS\0");
	m_deviceNameOverride = "";
	m_commandLength = 0;
	m_numericInputs.clear();
	m_numericOutputs.clear();
	m_queuedInputs.clear();
	m_events.clear();
	m_eventBuffer.clear();
}

//------------------------------------------------------------------------------

void OisHost::Poll(OIS_STRING_BUILDER& sb)
{
	ConnectAndPoll();

	for (int index : m_queuedOutputs)
	{
		const NumericValue& v = m_numericOutputs[index];
		switch (v.type)
		{
		case Boolean:  OIS_INFO("-> %d(%s) = %s",   v.channel, v.name.c_str(), v.value.boolean ? "true" : "false"); break;
		case Number:   OIS_INFO("-> %d(%s) = %d",   v.channel, v.name.c_str(), v.value.number);                     break;
		case Fraction: OIS_INFO("-> %d(%s) = %.2f", v.channel, v.name.c_str(), v.value.fraction);                   break;
		}
		SendValue(v, sb, CL_PAYLOAD_SHIFT, CL_VAL_1, CL_VAL_2, CL_VAL_3, CL_VAL_4);
	}
	m_queuedOutputs.clear();

	for (int index : m_eventBuffer)
	{
		const Event& e = m_events[index];

		OIS_INFO("-> EXC: %d (%s)", e.channel, e.name.c_str());
		if (m_binary)
		{
			const unsigned extraBits = 8 - CL_PAYLOAD_SHIFT;
			const unsigned channelLimit1 = 1U << extraBits;
			const unsigned channelLimit2 = 1U << (8 + extraBits);

			uint8_t cmd[3];
			int cmdLength;
			if (e.channel < channelLimit1)
			{
				cmdLength = 1;
				cmd[0] = CL_EXC_0 | (uint8_t)(e.channel << CL_PAYLOAD_SHIFT);
			}
			else if (e.channel < channelLimit2)
			{
				cmdLength = 2;
				cmd[0] = CL_EXC_1 | (uint8_t)((e.channel>>8) << CL_PAYLOAD_SHIFT);
				cmd[1] = (uint8_t)(e.channel & 0xFF);
			}
			else
			{
				cmdLength = 3;
				cmd[0] = CL_EXC_2;
				cmd[1] = (uint8_t)( e.channel     & 0xFF);
				cmd[2] = (uint8_t)((e.channel>>8) & 0xFF);
			}
			SendData((char*)cmd, cmdLength);
		}
		else
		{
			SendText(sb.FormatTemp("EXC=%d\n", e.channel));
		}
	}
	m_eventBuffer.clear();
}

int OisHost::ProcessBinary(char* start, char* end)
{
	if (end <= start)
		return 0;

	int bufferLength = (int)(end - start);
	
	uint32_t payload = (*start);
	int command = payload & SV_COMMAND_MASK;
	int cmdLength = 1;
	switch (command)
	{
		default:
		case SV_NUL:
			OIS_WARN( "Unknown command: 0x%x", payload);
			break;
		case SV_END_:
			break;
		case SV_VAL_1: cmdLength += 1; break;
		case SV_VAL_2: cmdLength += 2; break;
		case SV_VAL_3: cmdLength += 3; break;
		case SV_VAL_4: cmdLength += 4; break;
	}

	if (bufferLength < cmdLength)
		return 0;

	switch (command)
	{
		case SV_VAL_1:
		case SV_VAL_2:
		case SV_VAL_3:
		case SV_VAL_4:
		{
			if( !ExpectState( 1<<Active, "VAL", 2 ) )
				return false;
			int16_t channel = 0;
			uint32_t extra = payload >> SV_PAYLOAD_SHIFT;
			int16_t value = 0;
			switch( command )
			{
			default: OIS_ASSUME(false);
			case SV_VAL_1: value = extra;                              channel = *(uint8_t *)(start+1);              break;
			case SV_VAL_2: value = *(uint8_t *)(start+1)|(extra << 8); channel = *(uint8_t *)(start+2);              break;
			case SV_VAL_3: value = *(uint16_t*)(start+1);              channel = *(uint8_t *)(start+3)|(extra << 8); break;
			case SV_VAL_4: value = *(uint16_t*)(start+1);              channel = *(uint16_t*)(start+3);              break;
			}
			NumericValue* v = FindChannel(m_numericInputs, channel);
			if( v )
			{
				v->value = FromRawValue(v->type, value);
				switch( v->type )
				{
				case Boolean:  OIS_INFO( "<- %d(%s) = %s",   channel, v->name.c_str(), v->value.boolean ? "true" : "false" ); break;
				case Number:   OIS_INFO( "<- %d(%s) = %d",   channel, v->name.c_str(), v->value.number );                     break;
				case Fraction: OIS_INFO( "<- %d(%s) = %.2f", channel, v->name.c_str(), v->value.fraction );                   break;
				}
			}
			else
				OIS_WARN( "Received key/value message for unregistered channel %d", channel);
			break;
		}
		case SV_END_:
		{
			OIS_INFO( "<- END");
			if( m_connectionState != Handshaking )
				ClearState();
			m_port.Disconnect();
			break;
		}
	}

	return cmdLength;
}

bool OisHost::ProcessAscii(char* cmd, OIS_STRING_BUILDER& sb)
{
	if (!cmd[0])
		return false;
	uint32_t type = 0;
	if (cmd[1] && cmd[2])
		type = OIS_FOURCC(cmd);
	bool isKeyVal = 0 != isdigit(cmd[0]);
	if (isKeyVal)
	{
		if (!ExpectState(1 << Active, cmd, 2))
			return false;
		const char* payload = ZeroDelimiter(cmd, '=');
		int channel = atoi(cmd);
		NumericValue* v = FindChannel(m_numericInputs, channel);
		if (v)
		{
			v->value = FromRawValue(v->type, atoi(payload));
			switch (v->type)
			{
			case Boolean:  OIS_INFO("<- %d(%s) = %s",   channel, v->name.c_str(), v->value.boolean ? "true" : "false"); break;
			case Number:   OIS_INFO("<- %d(%s) = %d",   channel, v->name.c_str(), v->value.number);                     break;
			case Fraction: OIS_INFO("<- %d(%s) = %.2f", channel, v->name.c_str(), v->value.fraction);                   break;
			}
		}
		else
			OIS_WARN("Received key/value message for unregistered channel %d", channel);
	}
	else
	{
		char* payload = cmd + 4;
		switch (type)
		{
			default:
			{
				OIS_WARN( "Unknown command: %s", cmd);
				break;
			}
			case END:
			{
				OIS_INFO( "<- END");
				if( m_connectionState != Handshaking )
					ClearState();
				m_port.Disconnect();
				break;
			}
		}
	}
	return true;
}

bool OisHost::SetOutput(const NumericValue& output, Value value)
{
	return SetValueAndEnqueue(output, value, m_numericOutputs, m_queuedOutputs);
}

void OisHost::ClearState()
{
	m_connectionState = Handshaking;
	m_protocolVersion = 1;
	m_binary = false;
	m_gameVersion = 0;
	m_gameName = "";
	m_commandLength = 0;
	m_queuedOutputs.clear();
	m_eventBuffer.clear();
}

//------------------------------------------------------------------------------
#endif // OIS_DEVICE_IMPL
//------------------------------------------------------------------------------

#endif // OIS_DEVICE_INCLUDED