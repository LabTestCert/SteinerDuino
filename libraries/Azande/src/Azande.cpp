/* File: Azande.cpp
 *
 */

// --- Includes ---
#include "Azande.h"

// --- Defines ---

#define FIXED_ID 100

#define ID_SEND_FEATURES 			0x77
#define SUBID_COMMAND_ALL_FEATURES	0x01
#define MSG_ID_COMMAND_ALL_FEATURES	0x77, 0x01
#define MSG_ID_ONE_FEATURE			0x77, 0x02
#define MSG_ID_TARGET_INFO			0x77, 0x03
#define MSG_ID_FEATURE_LIST_CHANGED	0x77, 0x04

#define MSG_START 			0xAA
#define MSG_END 			0xBB
#define MSG_ESCAPE 			0xCC
#define MSG_ESCAPE_START 	0xDD
#define MSG_ESCAPE_END 		0xEE
#define MSG_ESCAPE_ESCAPE 	0xFF

#define CRC_SEED_VALUE 0xFFFF

#ifdef MCU_IS_LITTLE_ENDIAN
	#define CRC_LOW		1
	#define CRC_HIGH	0
#else
	#define CRC_LOW		0
	#define CRC_HIGH	1
#endif

typedef union
{
    byte hilo[2];
    unsigned short integer;
} INT_BYTE_FIELD_CRC;

typedef union
{
	long integer;
	byte bytes[4];
} INT32_UNION_T;

// TODO: move to local
typedef union
{
	double dValue;
	unsigned char dBuffer[4];
} DOUBLE_UNION_4_T;

// TODO: move to local
typedef union
{
	double dValue;
	unsigned char dBuffer[8];
} DOUBLE_UNION_8_T;

typedef void (*AZANDE_VOID_BOOL_FUNC_PTR_T)(boolean);
typedef void (*AZANDE_VOID_LONG_FUNC_PTR_T)(long);
typedef void (*AZANDE_VOID_DOUBLE_FUNC_PTR_T)(double);
typedef void (*AZANDE_VOID_TEXT_FUNC_PTR_T)(char*, unsigned int);

static const char msgNoPayload[1] __FLASH_DIRECTIVE = {0x00};

static const byte Tab_Crc16_Hi[256] __FLASH_DIRECTIVE =
{
	0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x81,0x91,0xA1,0xB1,0xC1,0xD1,0xE1,0xF1,
	0x12,0x02,0x32,0x22,0x52,0x42,0x72,0x62,0x93,0x83,0xB3,0xA3,0xD3,0xC3,0xF3,0xE3,
	0x24,0x34,0x04,0x14,0x64,0x74,0x44,0x54,0xA5,0xB5,0x85,0x95,0xE5,0xF5,0xC5,0xD5,
	0x36,0x26,0x16,0x06,0x76,0x66,0x56,0x46,0xB7,0xA7,0x97,0x87,0xF7,0xE7,0xD7,0xC7,
	0x48,0x58,0x68,0x78,0x08,0x18,0x28,0x38,0xC9,0xD9,0xE9,0xF9,0x89,0x99,0xA9,0xB9,
	0x5A,0x4A,0x7A,0x6A,0x1A,0x0A,0x3A,0x2A,0xDB,0xCB,0xFB,0xEB,0x9B,0x8B,0xBB,0xAB,
	0x6C,0x7C,0x4C,0x5C,0x2C,0x3C,0x0C,0x1C,0xED,0xFD,0xCD,0xDD,0xAD,0xBD,0x8D,0x9D,
	0x7E,0x6E,0x5E,0x4E,0x3E,0x2E,0x1E,0x0E,0xFF,0xEF,0xDF,0xCF,0xBF,0xAF,0x9F,0x8F,
	0x91,0x81,0xB1,0xA1,0xD1,0xC1,0xF1,0xE1,0x10,0x00,0x30,0x20,0x50,0x40,0x70,0x60,
	0x83,0x93,0xA3,0xB3,0xC3,0xD3,0xE3,0xF3,0x02,0x12,0x22,0x32,0x42,0x52,0x62,0x72,
	0xB5,0xA5,0x95,0x85,0xF5,0xE5,0xD5,0xC5,0x34,0x24,0x14,0x04,0x74,0x64,0x54,0x44,
	0xA7,0xB7,0x87,0x97,0xE7,0xF7,0xC7,0xD7,0x26,0x36,0x06,0x16,0x66,0x76,0x46,0x56,
	0xD9,0xC9,0xF9,0xE9,0x99,0x89,0xB9,0xA9,0x58,0x48,0x78,0x68,0x18,0x08,0x38,0x28,
	0xCB,0xDB,0xEB,0xFB,0x8B,0x9B,0xAB,0xBB,0x4A,0x5A,0x6A,0x7A,0x0A,0x1A,0x2A,0x3A,
	0xFD,0xED,0xDD,0xCD,0xBD,0xAD,0x9D,0x8D,0x7C,0x6C,0x5C,0x4C,0x3C,0x2C,0x1C,0x0C,
	0xEF,0xFF,0xCF,0xDF,0xAF,0xBF,0x8F,0x9F,0x6E,0x7E,0x4E,0x5E,0x2E,0x3E,0x0E,0x1E
};

static const byte Tab_Crc16_Lo[256] __FLASH_DIRECTIVE =
{
	0x00,0x21,0x42,0x63,0x84,0xA5,0xC6,0xE7,0x08,0x29,0x4A,0x6B,0x8C,0xAD,0xCE,0xEF,
	0x31,0x10,0x73,0x52,0xB5,0x94,0xF7,0xD6,0x39,0x18,0x7B,0x5A,0xBD,0x9C,0xFF,0xDE,
	0x62,0x43,0x20,0x01,0xE6,0xC7,0xA4,0x85,0x6A,0x4B,0x28,0x09,0xEE,0xCF,0xAC,0x8D,
	0x53,0x72,0x11,0x30,0xD7,0xF6,0x95,0xB4,0x5B,0x7A,0x19,0x38,0xDF,0xFE,0x9D,0xBC,
	0xC4,0xE5,0x86,0xA7,0x40,0x61,0x02,0x23,0xCC,0xED,0x8E,0xAF,0x48,0x69,0x0A,0x2B,
	0xF5,0xD4,0xB7,0x96,0x71,0x50,0x33,0x12,0xFD,0xDC,0xBF,0x9E,0x79,0x58,0x3B,0x1A,
	0xA6,0x87,0xE4,0xC5,0x22,0x03,0x60,0x41,0xAE,0x8F,0xEC,0xCD,0x2A,0x0B,0x68,0x49,
	0x97,0xB6,0xD5,0xF4,0x13,0x32,0x51,0x70,0x9F,0xBE,0xDD,0xFC,0x1B,0x3A,0x59,0x78,
	0x88,0xA9,0xCA,0xEB,0x0C,0x2D,0x4E,0x6F,0x80,0xA1,0xC2,0xE3,0x04,0x25,0x46,0x67,
	0xB9,0x98,0xFB,0xDA,0x3D,0x1C,0x7F,0x5E,0xB1,0x90,0xF3,0xD2,0x35,0x14,0x77,0x56,
	0xEA,0xCB,0xA8,0x89,0x6E,0x4F,0x2C,0x0D,0xE2,0xC3,0xA0,0x81,0x66,0x47,0x24,0x05,
	0xDB,0xFA,0x99,0xB8,0x5F,0x7E,0x1D,0x3C,0xD3,0xF2,0x91,0xB0,0x57,0x76,0x15,0x34,
	0x4C,0x6D,0x0E,0x2F,0xC8,0xE9,0x8A,0xAB,0x44,0x65,0x06,0x27,0xC0,0xE1,0x82,0xA3,
	0x7D,0x5C,0x3F,0x1E,0xF9,0xD8,0xBB,0x9A,0x75,0x54,0x37,0x16,0xF1,0xD0,0xB3,0x92,
	0x2E,0x0F,0x6C,0x4D,0xAA,0x8B,0xE8,0xC9,0x26,0x07,0x64,0x45,0xA2,0x83,0xE0,0xC1,
	0x1F,0x3E,0x5D,0x7C,0x9B,0xBA,0xD9,0xF8,0x17,0x36,0x55,0x74,0x93,0xB2,0xD1,0xF0
};


Azande::Azande():
myStream(Serial)
{

}

Azande::Azande(Stream& useStream):
myStream(useStream)
{

}

void Azande::begin()
{
}

void Azande::begin(const char* info)
{
	targetInfo = info;
}

void Azande::readStream()
{
	byte data;

	while (myStream.available() > 0)
	{
		data = (byte)myStream.read();
		if (data == MSG_START)
		{
			rawInBufferIndex = 0;
			isStartByteAccepted = true;
		}
		else if (data == MSG_END)
		{
			if (isStartByteAccepted)
			{
				evaluateMessage();
				isStartByteAccepted = false;
				return;
			}
			isStartByteAccepted = false;
		}
		else
		{
			rawInBuffer[rawInBufferIndex] = data;

			++rawInBufferIndex;
			if (rawInBufferIndex >= READ_BUFFER_SIZE) // the message was too long
			{
				rawInBufferIndex = 0;
				isStartByteAccepted = false;
			}
		}
	}

	return;
}

void Azande::add(const AZANDE_COMMAND* cmd_ptr)
{
	command_list[command_count] = cmd_ptr;
	++command_count;
}

void Azande::add(const AZANDE_EVENT_BOOL* event_ptr)
{
	event_msg_list[event_msg_count] = &(event_ptr->msg);
	++event_msg_count;
}

void Azande::add(const AZANDE_EVENT_INT* event_ptr)
{
	event_msg_list[event_msg_count] = &(event_ptr->msg);
	++event_msg_count;
}

void Azande::add(const AZANDE_EVENT_DOUBLE* event_ptr)
{
	event_msg_list[event_msg_count] = &(event_ptr->msg);
	++event_msg_count;
}

void Azande::add(const AZANDE_EVENT_TEXT* event_ptr)
{
	event_msg_list[event_msg_count] = &(event_ptr->msg);
	++event_msg_count;
}


void Azande::clear()
{
	command_count = 0;
	event_msg_count = 0;
}


void Azande::notify()
{
	sendOneStaticMessage(msgNoPayload, MSG_ID_FEATURE_LIST_CHANGED);
}

boolean Azande::tryBuildHeader(byte id, byte subId)
{
	unsigned char i;

	for (i = 0; i < event_msg_count; i++)
	{
		if ((id == __FLASH_READ_BYTE(event_msg_list[i]->id)) && (subId == __FLASH_READ_BYTE(event_msg_list[i]->subId)))
		{
			// Start byte
			WriteOneByteQueued(0xAA);

			sendCrc = CRC_SEED_VALUE;

			// Message Id
			writeWithEscape(id);
			writeWithEscape(subId);

			return true;
		}
	}

	return false;
}

void Azande::sendMessageTail(void)
{
	byte crc_msb;
	byte crc_lsb;

	// CRC
	crc_msb = sendCrc / 256;
	crc_lsb = sendCrc % 256;

	writeWithEscape(crc_msb);
	writeWithEscape(crc_lsb);

	// End byte
	WriteOneByteQueued(0xBB);
}

void Azande::send(const AZANDE_EVENT_BOOL* event_ptr, boolean isTrue)
{

	if (tryBuildHeader(__FLASH_READ_BYTE(event_ptr->msg.id), __FLASH_READ_BYTE(event_ptr->msg.subId)))
	{
		// Payload
		if (isTrue)
		{
			writeWithEscape(1);
		}
		else
		{
			writeWithEscape(0);
		}

		sendMessageTail();
	}
}

void Azande::send(const AZANDE_EVENT_INT* event_ptr, long value)
{
	INT32_UNION_T unionen;

	if (tryBuildHeader(__FLASH_READ_BYTE(event_ptr->msg.id), __FLASH_READ_BYTE(event_ptr->msg.subId)))
	{
		unionen.integer = value;

		// Payload
		// The value is sent as little endian
	#ifdef MCU_IS_LITTLE_ENDIAN
		// Little => Big (reverse order)
		writeWithEscape(unionen.bytes[3]);
		writeWithEscape(unionen.bytes[2]);
		writeWithEscape(unionen.bytes[1]);
		writeWithEscape(unionen.bytes[0]);
	#else
		// Big => Big
		writeWithEscape(unionen.bytes[0]);
		writeWithEscape(unionen.bytes[1]);
		writeWithEscape(unionen.bytes[2]);
		writeWithEscape(unionen.bytes[3]);
	#endif

		sendMessageTail();
	}
}

void Azande::send(const AZANDE_EVENT_TEXT* event_ptr, char* text)
{
	unsigned int i;

	if (tryBuildHeader(__FLASH_READ_BYTE(event_ptr->msg.id), __FLASH_READ_BYTE(event_ptr->msg.subId)))
	{

		// Payload
		for (i = 0; text[i] != 0; i++)
		{
			writeWithEscape(text[i]);
		}

		sendMessageTail();
	}
}

void Azande::send(const AZANDE_EVENT_DOUBLE* event_ptr, double value)
{
	int i;

	if (tryBuildHeader(__FLASH_READ_BYTE(event_ptr->msg.id), __FLASH_READ_BYTE(event_ptr->msg.subId)))
	{
		// Payload
		if (sizeof(double) == 8) // Double precision (8-bytes)
		{
			memcpy(tempBuffer, &value, 8);
		}
		else if (sizeof(double) == 4) // Single precision (4-bytes)
		{
			convertSingle2DoublePrecisionArray(tempBuffer, value);
		}
		else
		{
			// Not supported
			for (i = 0; i < 8; i++)
			{
				tempBuffer[i] = 0;
			}
		}

	#ifdef MCU_IS_LITTLE_ENDIAN
		// Little => Big (reverse order)
		for (i = 7; i >= 0; i--)
		{
			writeWithEscape(tempBuffer[i]);
		}
	#else
		// Big => Big (keep order)
		for (i = 0; i < 8; i++)
		{
			writeWithEscape(tempBuffer[i]);
		}
	#endif

		sendMessageTail();
	}
}

// --- Private functions ---

void Azande::evaluateMessage(void)
{
	if (rawInBufferIndex > 3)
	{
		if (unescapeBuffer() == true)
		{
			if (isCrcOk() == true)
			{
				exeMatchingCommand();
			}
		}
	}
}


void Azande::sendAllFeatures(void)
{
	uint16_t i;

	if (targetInfo != 0)
	{
		sendOneStaticMessage(targetInfo, MSG_ID_TARGET_INFO);
	}

	for (i = 0; i < command_count; i++)
	{
		sendOneStaticMessage(__FLASH_READ_PTR(command_list[i]->xml), MSG_ID_ONE_FEATURE);
	}

	for (i = 0; i < event_msg_count; i++)
	{
		sendOneStaticMessage(__FLASH_READ_PTR(event_msg_list[i]->xml), MSG_ID_ONE_FEATURE);
	}
}


void Azande::sendOneStaticMessage(const char* data, byte id, byte subId)
{
	byte crc_msb;
	byte crc_lsb;
	const char* data_ptr = data;

	// Start byte
	WriteOneByteBlocked(0xAA);

	sendCrc = 0xFFFF;

	// Message Id
	blockWriteWithEscape(id);

	// Message Sub Id
	blockWriteWithEscape(subId);

	// Payload
	while (__FLASH_READ_BYTE(*data_ptr) != 0)
	{
		blockWriteWithEscape(__FLASH_READ_BYTE(*data_ptr));
		++data_ptr;
	}

	// CRC
	crc_msb = sendCrc / 256;
	crc_lsb = sendCrc % 256;

	blockWriteWithEscape(crc_msb);
	blockWriteWithEscape(crc_lsb);

	// End byte
	WriteOneByteBlocked(0xBB);

}

boolean Azande::unescapeBuffer(void)
{
	boolean escapeFound = false;
	uint16_t i;

	unescapedInIndex = 0;

	for (i = 0; i < rawInBufferIndex; i++)
	{
		if (escapeFound == true)
		{
			escapeFound = false;

			if (rawInBuffer[i] == MSG_ESCAPE_START)
			{
				rawInBuffer[unescapedInIndex] = MSG_START;
				++unescapedInIndex;
			}
			else if (rawInBuffer[i] == MSG_ESCAPE_END)
			{
				rawInBuffer[unescapedInIndex] = MSG_END;
				++unescapedInIndex;
			}
			else if (rawInBuffer[i] == MSG_ESCAPE_ESCAPE)
			{
				rawInBuffer[unescapedInIndex] = MSG_ESCAPE;
				++unescapedInIndex;
			}
			else
			{
				// Incorrect format found. Escape followed of a byte not MSG_ESCAPE_START, MSG_ESCAPE_END or MSG_ESCAPE_ESCAPE
				unescapedInIndex = 0;
				return false;
			}
		}
		else if (rawInBuffer[i] == MSG_ESCAPE)
		{
			escapeFound = true;
		}
		else
		{
			rawInBuffer[unescapedInIndex] = rawInBuffer[i];
			++unescapedInIndex;
		}
	}
	return true;
}


boolean Azande::isCrcOk(void)
{
	uint16_t i;
	uint16_t crc = 0xFFFF;

	for (i = 0; i < (unescapedInIndex - 2); i++)
	{
		crc = Crc_Update(crc, rawInBuffer[i]);
	}

	if ((rawInBuffer[unescapedInIndex - 1] == (crc % 256)) && (rawInBuffer[unescapedInIndex - 2] == (crc / 256)))
	{
		return true;
	}
	else
	{
		return false;
	}
}


void Azande::exeMatchingCommand(void)
{
	byte i;
	byte id;
	byte subId;

	id = rawInBuffer[0];
	subId = rawInBuffer[1];

	if ((id == ID_SEND_FEATURES) && (subId == SUBID_COMMAND_ALL_FEATURES))
	{
		sendAllFeatures();
	}
	else
	{
		for (i = 0; i < command_count; i++)
		{
			if ((id == __FLASH_READ_BYTE(command_list[i]->id)) && (subId == __FLASH_READ_BYTE(command_list[i]->subId)))
			{
				switch (__FLASH_READ_BYTE(command_list[i]->type))
				{
				case AZANDE_COMMAND_TYPE_VOID:
					((AZANDE_VOID_VOID_FUNC_PTR_T)(__FLASH_READ_PTR(command_list[i]->cmd_function)))();
					break;

				case AZANDE_COMMAND_TYPE_BOOL:
					((AZANDE_VOID_BOOL_FUNC_PTR_T)(__FLASH_READ_PTR(command_list[i]->cmd_function)))(rawInBuffer[2] == 1);
					break;

				case AZANDE_COMMAND_TYPE_INT:
				case AZANDE_COMMAND_TYPE_ENUM:
					((AZANDE_VOID_LONG_FUNC_PTR_T)(__FLASH_READ_PTR(command_list[i]->cmd_function)))(getIntValue(&rawInBuffer[2]));
					break;

				case AZANDE_COMMAND_TYPE_DOUBLE:
					((AZANDE_VOID_DOUBLE_FUNC_PTR_T)(__FLASH_READ_PTR(command_list[i]->cmd_function)))(getDoubleValue(&rawInBuffer[2]));
					break;

				case AZANDE_COMMAND_TYPE_TEXT:
					((AZANDE_VOID_TEXT_FUNC_PTR_T)(__FLASH_READ_PTR(command_list[i]->cmd_function)))((char*)&(rawInBuffer[2]), unescapedInIndex - 4);
					break;

				}
			}
		}
	}
}

void Azande::writeWithEscape(byte data)
{
	byte new_data;

	new_data = data;

	sendCrc = Crc_Update(sendCrc, data);

	if (data == MSG_START)
	{
		WriteOneByteQueued(MSG_ESCAPE);
		new_data = MSG_ESCAPE_START;
	}
	else if (data == MSG_END)
	{
		WriteOneByteQueued(MSG_ESCAPE);
		new_data = MSG_ESCAPE_END;
	}
	else if (data == MSG_ESCAPE)
	{
		WriteOneByteQueued(MSG_ESCAPE);
		new_data = MSG_ESCAPE_ESCAPE;
	}

	WriteOneByteQueued(new_data);

}



void Azande::blockWriteWithEscape(byte data)
{
	byte new_data;

	new_data = data;

	sendCrc = Crc_Update(sendCrc, data);

	if (data == MSG_START)
	{
		WriteOneByteBlocked(MSG_ESCAPE);
		new_data = MSG_ESCAPE_START;
	}
	else if (data == MSG_END)
	{
		WriteOneByteBlocked(MSG_ESCAPE);
		new_data = MSG_ESCAPE_END;
	}
	else if (data == MSG_ESCAPE)
	{
		WriteOneByteBlocked(MSG_ESCAPE);
		new_data = MSG_ESCAPE_ESCAPE;
	}

	WriteOneByteBlocked(new_data);

}


void Azande::convertSingle2DoublePrecisionArray(byte* destination_array, const double value)
{
	volatile DOUBLE_UNION_4_T Double_Union_4_Value;
	Double_Union_4_Value.dValue = value;

	memset(destination_array, 0, 8);

#ifdef MCU_IS_LITTLE_ENDIAN
	// Convert and keep as little endian
	destination_array[7] = ((Double_Union_4_Value.dBuffer[3]  >> 3) & 0x07); 	// bit 27-29 ==>> bit 56-58
	destination_array[7] |= (Double_Union_4_Value.dBuffer[3] & 0xC0); 			// bit 30-31 ==>> bit 62-63
	if ((Double_Union_4_Value.dValue < 2) && (Double_Union_4_Value.dValue > -2))
	{
		destination_array[7] |= 0x38; 											// set bit 59-60
	}
	destination_array[6] = (Double_Union_4_Value.dBuffer[3] << 5);				// bit 24-29 ==>> bit 53-55
	destination_array[6] |= (Double_Union_4_Value.dBuffer[2] >> 3);				// bit 19-23 ==>> bit 48-52
	destination_array[5] = (Double_Union_4_Value.dBuffer[2] << 5);				// bit 16-18 ==>> bit 45-47
	destination_array[5] |= (Double_Union_4_Value.dBuffer[1] >> 3);				// bit 11-15 ==>> bit 40-44
	destination_array[4] = (Double_Union_4_Value.dBuffer[1] << 5);				// bit 8 -10 ==>> bit 37-39
	destination_array[4] |= (Double_Union_4_Value.dBuffer[0] >> 3);				// bit 3 - 7 ==>> bit 32-36
	destination_array[3] = (Double_Union_4_Value.dBuffer[0] << 5);				// bit 0 - 2 ==>> bit 29-31
	destination_array[2] = 0;													// clear
	destination_array[1] = 0;													// clear
	destination_array[0] = 0;													// clear
#else // Big Endian
	// Convert and keep as big endian
	destination_array[0] = ((Double_Union_4_Value.dBuffer[0]  >> 3) & 0x07); // MSB
	destination_array[0] |= (Double_Union_4_Value.dBuffer[0] & 0xC0);
	if ((Double_Union_4_Value.dValue < 2) && (Double_Union_4_Value.dValue > -2))
	{
		destination_array[0] |= 0x38; 											// set bit 59-60
	}
	destination_array[1] = (Double_Union_4_Value.dBuffer[0] << 5);
	destination_array[1] += (Double_Union_4_Value.dBuffer[1] >> 3);
	destination_array[2] = (Double_Union_4_Value.dBuffer[1] << 5);
	destination_array[2] += (Double_Union_4_Value.dBuffer[2] >> 3);
	destination_array[3] = (Double_Union_4_Value.dBuffer[2] << 5);
	destination_array[3] += (Double_Union_4_Value.dBuffer[3] >> 3);
	destination_array[4] = (Double_Union_4_Value.dBuffer[3] << 5);
	destination_array[5] = 0;
	destination_array[6] = 0;
	destination_array[7] = 0;											// LSB
#endif

}


double Azande::convertDoublePrecisionArray2Single(byte* big_endian_source_array)
{
	volatile DOUBLE_UNION_4_T Double_Union_4_Value;

#ifdef MCU_IS_LITTLE_ENDIAN
	// Little => Big (reverse order)
	Double_Union_4_Value.dBuffer[3] = (big_endian_source_array[1] >> 5); // MSB
	Double_Union_4_Value.dBuffer[3] |= ((big_endian_source_array[0] & 0x07 ) << 3); // MSB
	Double_Union_4_Value.dBuffer[3] |= (big_endian_source_array[0] & 0xC0); // MSB

	Double_Union_4_Value.dBuffer[2] = (big_endian_source_array[2] >> 5);
	Double_Union_4_Value.dBuffer[2] += (big_endian_source_array[1] << 3);

	Double_Union_4_Value.dBuffer[1] = (big_endian_source_array[3] >> 5);
	Double_Union_4_Value.dBuffer[1] += (big_endian_source_array[2] << 3);

	Double_Union_4_Value.dBuffer[0] = (big_endian_source_array[4] >> 5); // LSB
	Double_Union_4_Value.dBuffer[0] += (big_endian_source_array[3] << 3); // LSB
#else
	// Big => Big (keep order)
	Double_Union_4_Value.dBuffer[0] = (big_endian_source_array[1] >> 5); // MSB
	Double_Union_4_Value.dBuffer[0] |= ((big_endian_source_array[0] & 0x07 ) << 3); // MSB
	Double_Union_4_Value.dBuffer[0] |= (big_endian_source_array[0] & 0xC0); // MSB

	Double_Union_4_Value.dBuffer[1] = (big_endian_source_array[2] >> 5);
	Double_Union_4_Value.dBuffer[1] += (big_endian_source_array[1] << 3);

	Double_Union_4_Value.dBuffer[2] = (big_endian_source_array[3] >> 5);
	Double_Union_4_Value.dBuffer[2] += (big_endian_source_array[2] << 3);

	Double_Union_4_Value.dBuffer[3] = (big_endian_source_array[4] >> 5); // LSB
	Double_Union_4_Value.dBuffer[3] += (big_endian_source_array[3] << 3); // LSB
#endif

	return Double_Union_4_Value.dValue;
}


double Azande::getDoubleValue(byte* big_endian_source_array)
{
	byte i;
	volatile DOUBLE_UNION_8_T Double_Union_8_Value;

	if (sizeof(double) == 8)
	{
#ifdef MCU_IS_LITTLE_ENDIAN
		// Little => Big (reverse order)
		for (i = 0; i < 8; i++)
		{
			Double_Union_8_Value.dBuffer[i] = big_endian_source_array[7 - i];
		}
#else
		// Big => Big (keep order)
		for (i = 0; i < 8; i++)
		{
			Double_Union_8_Value.dBuffer[i] = big_endian_source_array[i];
		}
#endif
		return Double_Union_8_Value.dValue;
	}
	else if (sizeof(double) == 4)
	{
		return convertDoublePrecisionArray2Single(big_endian_source_array);
	}
	else
	{
		// Not supported
		return 0.0;
	}
}


long Azande::getIntValue(byte* big_endian_source_array)
{
	byte i;
	volatile INT32_UNION_T int_union;

#ifdef MCU_IS_LITTLE_ENDIAN
		// Little => Big (reverse order)
		for (i = 0; i < 4; i++)
		{
			int_union.bytes[i] = big_endian_source_array[3 - i];
		}
#else
		// Big => Big (keep order)
		for (i = 0; i < 4; i++)
		{
			int_union.bytes[i] = big_endian_source_array[i];
		}
#endif
		return int_union.integer;

}


void Azande::WriteOneByteQueued(byte data)
{
	myStream.write(data);
}


void Azande::WriteOneByteBlocked(byte data)
{
	while (myStream.write(data) == 0);
}


//------------------------------------------------------------------------------
/*!
 *  \brief      Updates the CRC accumulator processing the new
 *              piece (one byte) of data provided in data_byte.
 *              The function uses the CRC16-CCITT table, a 256-
 *              word table containing all CRC remainders for every
 *              possible byte.
 *              CRC-CCITT Polynomial 0x1021 uses seed 0xFFFF
 *
 *  \param      crc_accum: the running crc computed on the bytes already processed.  It must be initialized to 0xFFFF
 *  \param		data_byte: the current data byte for which the crc must be updated.
 *  \return     The function returns the updated CRC
 */
//------------------------------------------------------------------------------
unsigned int Azande::Crc_Update(unsigned int crc_accum, byte data)
{
    INT_BYTE_FIELD_CRC reg;
    byte data_byte;

    reg.integer = crc_accum;
    data_byte = data ^ reg.hilo[CRC_LOW];
    reg.hilo[CRC_LOW] = reg.hilo[CRC_HIGH] ^ __FLASH_READ_BYTE(Tab_Crc16_Hi[data_byte]);
    reg.hilo[CRC_HIGH] = __FLASH_READ_BYTE(Tab_Crc16_Lo[data_byte]);
    return reg.integer;
}


