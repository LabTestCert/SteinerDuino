/* File Azande.h
 *
 */
#ifndef AZANDE_H
#define AZANDE_H

#include "Arduino.h"
#include "Stream.h"
#include "AzandeSystemConfig.h"
#include "AzandeUserMacros.h"


//==============================================================================
//--------------------------- Public Defines, Typedef --------------------------
//==============================================================================

typedef enum
{
	AZANDE_COMMAND_TYPE_VOID,
	AZANDE_COMMAND_TYPE_BOOL,
	AZANDE_COMMAND_TYPE_INT,
	AZANDE_COMMAND_TYPE_ENUM,
	AZANDE_COMMAND_TYPE_DOUBLE,
	AZANDE_COMMAND_TYPE_TEXT,
}AZANDE_COMMAND_TYPE;

typedef enum
{
	AZANDE_EVENT_TYPE_BOOL,
	AZANDE_EVENT_TYPE_INT,
	AZANDE_EVENT_TYPE_DOUBLE,
	AZANDE_EVENT_TYPE_TEXT,
}AZANDE_EVENT_TYPE;

typedef void (*AZANDE_VOID_VOID_FUNC_PTR_T)(void);

typedef struct
{
	unsigned char id;
	unsigned char subId;
	const char* xml;
	AZANDE_COMMAND_TYPE type;
	AZANDE_VOID_VOID_FUNC_PTR_T cmd_function;
}AZANDE_COMMAND;

typedef struct
{
	unsigned char id;
	unsigned char subId;
	const char* xml;
}AZANDE_MSG;

typedef struct
{
	AZANDE_MSG msg;
}AZANDE_EVENT_BOOL;

typedef struct
{
	AZANDE_MSG msg;
}AZANDE_EVENT_INT;

typedef struct
{
	AZANDE_MSG msg;
}AZANDE_EVENT_DOUBLE;

typedef struct
{
	AZANDE_MSG msg;
}AZANDE_EVENT_TEXT;


//==============================================================================
//--------------------------- Class --------------------------------------------
//==============================================================================
class Azande
{
  public:
	Azande();
	Azande(Stream& useStream);
	void begin();
	void begin(const char* info);
	void readStream();
    void add(const AZANDE_COMMAND* cmd_ptr);
    void add(const AZANDE_EVENT_BOOL* event_ptr);
    void add(const AZANDE_EVENT_INT* event_ptr);
    void add(const AZANDE_EVENT_DOUBLE* event_ptr);
    void add(const AZANDE_EVENT_TEXT* event_ptr);
    void clear();
    void notify();
    void send(const AZANDE_EVENT_BOOL* event_ptr, boolean isTrue);
    void send(const AZANDE_EVENT_INT* event_ptr, long value);
    void send(const AZANDE_EVENT_DOUBLE* event_ptr, double value);
    void send(const AZANDE_EVENT_TEXT* event_ptr, char* text);

  private:
	// Member Variables
    Stream& myStream;
    byte rawInBuffer[READ_BUFFER_SIZE];
	byte tempBuffer[8];
	unsigned int rawInBufferIndex;
	unsigned int unescapedInIndex;
	unsigned int sendCrc;
	boolean isStartByteAccepted;
	AZANDE_COMMAND const* command_list[AZANDE_MAX_COMMANDS];
	unsigned int command_count = 0;
	char const* targetInfo = 0;
	AZANDE_MSG const* event_msg_list[AZANDE_MAX_EVENTS];
	unsigned int event_msg_count = 0;

	// Member Functions
	void evaluateMessage(void);
	boolean unescapeBuffer(void);
	boolean isCrcOk(void);
	void exeMatchingCommand(void);
	void writeWithEscape(byte data);
	void blockWriteWithEscape(byte data);
	boolean tryBuildHeader(byte id, byte subId);
	void sendMessageTail(void);
	void sendAllFeatures(void);
	void sendOneStaticMessage(const char* data, byte id, byte subId);
	void convertSingle2DoublePrecisionArray(byte* destination_array, const double value);
	double convertDoublePrecisionArray2Single(uint8_t* big_endian_source_array);
	double getDoubleValue(byte* big_endian_source_array);
	long getIntValue(byte* big_endian_source_array);
	void WriteOneByteQueued(byte data);
	void WriteOneByteBlocked(byte data);
	unsigned int Crc_Update(unsigned int crc_accum, byte data_byte);
};

#endif
