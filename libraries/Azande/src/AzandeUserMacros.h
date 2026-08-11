/* File: AzandeUserMacros.h
*/
#ifndef AZANDEUSERMACROS_H_
#define AZANDEUSERMACROS_H_


// Common macros ------------------------------------------
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AZANDE_PROTOCOL_VERSION_MAJOR "1"
#define AZANDE_PROTOCOL_VERSION_MINOR "0"

// Target Info ------------------------------------------
#define define_target_info(VAR_NAME, NAME, SW_VERSION, DATE, NOTE)\
static const char VAR_NAME[] __FLASH_DIRECTIVE =  "<Info><P><Mj>" AZANDE_PROTOCOL_VERSION_MAJOR "</Mj><Mi>" AZANDE_PROTOCOL_VERSION_MINOR "</Mi></P><N>" NAME "</N><V>" SW_VERSION "</V><D>" DATE "</D><Nt>" NOTE "</Nt></Info>";\

// void Command ------------------------------------------
#define define_void_command(NAME, TITLE, FUNCTION, GROUP)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><C><I>100</I><Is>" TOSTRING(__LINE__) "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr></C></F>";\
void FUNCTION(void);\
static const AZANDE_COMMAND NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	100,\
	__LINE__,\
	AZANDE_XML_STRING_##NAME,\
	AZANDE_COMMAND_TYPE_VOID,\
	(AZANDE_VOID_VOID_FUNC_PTR_T)FUNCTION\
};\
static const AZANDE_COMMAND* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


#define define_void_command_id(NAME, TITLE, FUNCTION, ID, SUBID, GROUP)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><C><I>" #ID "</I><Is>" #SUBID "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr></C></F>";\
void FUNCTION(void);\
static const AZANDE_COMMAND NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	ID,\
	SUBID,\
	AZANDE_XML_STRING_##NAME,\
	AZANDE_COMMAND_TYPE_VOID,\
	(AZANDE_VOID_VOID_FUNC_PTR_T)FUNCTION\
};\
static const AZANDE_COMMAND* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;



// Bool Command ------------------------------------------
#define define_bool_command(NAME, TITLE, FUNCTION, GROUP, TRUE_TXT, FALSE_TXT)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><Cb><I>100</I><Is>" TOSTRING(__LINE__) "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><B1>" TRUE_TXT "</B1><B0>" FALSE_TXT "</B0></Cb></F>";\
void FUNCTION(boolean);\
static const AZANDE_COMMAND NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	100,\
	__LINE__,\
	AZANDE_XML_STRING_##NAME,\
	AZANDE_COMMAND_TYPE_BOOL,\
	(AZANDE_VOID_VOID_FUNC_PTR_T)FUNCTION\
};\
static const AZANDE_COMMAND* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


#define define_bool_command_id(NAME, TITLE, FUNCTION, ID, SUBID, GROUP, TRUE_TXT, FALSE_TXT)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><Cb><I>" #ID "</I><Is>" #SUBID "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><B1>" TRUE_TXT "</B1><B0>" FALSE_TXT "</B0></Cb></F>";\
void FUNCTION(boolean);\
static const AZANDE_COMMAND NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	ID,\
	SUBID,\
	AZANDE_XML_STRING_##NAME,\
	AZANDE_COMMAND_TYPE_BOOL,\
	(AZANDE_VOID_VOID_FUNC_PTR_T)FUNCTION\
};\
static const AZANDE_COMMAND* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


// Int Command ------------------------------------------
#define define_int_command(NAME, TITLE, FUNCTION, GROUP, UNIT, MIN, MAX)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><Ci><I>100</I><Is>" TOSTRING(__LINE__) "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><U>" UNIT "</U><Mn>" #MIN "</Mn><Mx>" #MAX "</Mx></Ci></F>";\
void FUNCTION(long);\
static const AZANDE_COMMAND NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	100,\
	__LINE__,\
	AZANDE_XML_STRING_##NAME,\
	AZANDE_COMMAND_TYPE_INT,\
	(AZANDE_VOID_VOID_FUNC_PTR_T)FUNCTION\
};\
static const AZANDE_COMMAND* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


#define define_int_command_id(NAME, TITLE, FUNCTION, ID, SUBID, GROUP, UNIT, MIN, MAX)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><Ci><I>" #ID "</I><Is>" #SUBID "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><U>" UNIT "</U><Mn>" #MIN "</Mn><Mx>" #MAX "</Mx></Ci></F>";\
void FUNCTION(long);\
static const AZANDE_COMMAND NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	ID,\
	SUBID,\
	AZANDE_XML_STRING_##NAME,\
	AZANDE_COMMAND_TYPE_INT,\
	(AZANDE_VOID_VOID_FUNC_PTR_T)FUNCTION\
};\
static const AZANDE_COMMAND* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


// Enum Command ------------------------------------------
#define define_enum_item(VALUE, TXT)"<p><i>" #VALUE "</i><n>" TXT "</n></p>"

#define define_enum_command(NAME, TITLE, FUNCTION, GROUP, VALUES)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE = "<F><Ce><I>100</I><Is>" TOSTRING(__LINE__) "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><ps>" VALUES "</ps></Ce></F>";\
void FUNCTION(long);\
static const AZANDE_COMMAND NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	100,\
	__LINE__,\
	AZANDE_XML_STRING_##NAME,\
	AZANDE_COMMAND_TYPE_ENUM,\
	(AZANDE_VOID_VOID_FUNC_PTR_T)FUNCTION\
};\
static const AZANDE_COMMAND* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


#define define_enum_command_id(NAME, TITLE, FUNCTION, ID, SUBID, GROUP, VALUES)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE = "<F><Ce><I>" #ID "</I><Is>" #SUBID "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><ps>" VALUES "</ps></Ce></F>";\
void FUNCTION(long);\
static const AZANDE_COMMAND NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	ID,\
	SUBID,\
	AZANDE_XML_STRING_##NAME,\
	AZANDE_COMMAND_TYPE_ENUM,\
	(AZANDE_VOID_VOID_FUNC_PTR_T)FUNCTION\
};\
static const AZANDE_COMMAND* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


// Double Command ------------------------------------------
#define define_double_command(NAME, TITLE, FUNCTION, GROUP, UNIT, MIN, MAX)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><Cd><I>100</I><Is>" TOSTRING(__LINE__) "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><U>" UNIT "</U><Mn>" #MIN "</Mn><Mx>" #MAX "</Mx></Cd></F>";\
void FUNCTION(double);\
static const AZANDE_COMMAND NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	100,\
	__LINE__,\
	AZANDE_XML_STRING_##NAME,\
	AZANDE_COMMAND_TYPE_DOUBLE,\
	(AZANDE_VOID_VOID_FUNC_PTR_T)FUNCTION\
};\
static const AZANDE_COMMAND* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


#define define_double_command_id(NAME, TITLE, FUNCTION, ID, SUBID, GROUP, UNIT, MIN, MAX)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><Cd><I>" #ID "</I><Is>" #SUBID "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><U>" UNIT "</U><Mn>" #MIN "</Mn><Mx>" #MAX "</Mx></Cd></F>";\
void FUNCTION(double);\
static const AZANDE_COMMAND NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	ID,\
	SUBID,\
	AZANDE_XML_STRING_##NAME,\
	AZANDE_COMMAND_TYPE_DOUBLE,\
	(AZANDE_VOID_VOID_FUNC_PTR_T)FUNCTION\
};\
static const AZANDE_COMMAND* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


// Text Command ------------------------------------------
#define define_text_command(NAME, TITLE, FUNCTION, GROUP, MAX_LEN)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><Ct><I>100</I><Is>" TOSTRING(__LINE__) "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><Mx>" #MAX_LEN "</Mx></Ct></F>";\
void FUNCTION(char*, unsigned int);\
static const AZANDE_COMMAND NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	100,\
	__LINE__,\
	AZANDE_XML_STRING_##NAME,\
	AZANDE_COMMAND_TYPE_TEXT,\
	(AZANDE_VOID_VOID_FUNC_PTR_T)FUNCTION\
};\
static const AZANDE_COMMAND* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


#define define_text_command_id(NAME, TITLE, FUNCTION, ID, SUBID, GROUP, MAX_LEN)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><Ct><I>" #ID "</I><Is>" #SUBID "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><Mx>" #MAX_LEN "</Mx></Ct></F>";\
void FUNCTION(char*, unsigned int);\
static const AZANDE_COMMAND NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	ID,\
	SUBID,\
	AZANDE_XML_STRING_##NAME,\
	AZANDE_COMMAND_TYPE_TEXT,\
	(AZANDE_VOID_VOID_FUNC_PTR_T)FUNCTION\
};\
static const AZANDE_COMMAND* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


// Bool Event ------------------------------------------
#define define_bool_event(NAME, TITLE, GROUP, TRUE_TXT, FALSE_TXT)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><Eb><I>100</I><Is>" TOSTRING(__LINE__) "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><B1>" TRUE_TXT "</B1><B0>" FALSE_TXT "</B0></Eb></F>";\
static const AZANDE_EVENT_BOOL NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	100,\
	__LINE__,\
	AZANDE_XML_STRING_##NAME,\
};\
static const AZANDE_EVENT_BOOL* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


#define define_bool_event_id(NAME, TITLE, ID, SUBID, GROUP, TRUE_TXT, FALSE_TXT)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><Eb><I>" #ID "</I><Is>" #SUBID "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><B1>" TRUE_TXT "</B1><B0>" FALSE_TXT "</B0></Eb></F>";\
static const AZANDE_EVENT_BOOL NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	ID,\
	SUBID,\
	AZANDE_XML_STRING_##NAME,\
};\
static const AZANDE_EVENT_BOOL* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


// Int Event ------------------------------------------
#define define_int_event(NAME, TITLE, GROUP, UNIT, MIN, MAX, FORMAT)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><Ei><I>100</I><Is>" TOSTRING(__LINE__) "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><U>" UNIT "</U><Mn>" #MIN "</Mn><Mx>" #MAX "</Mx><Vf>" FORMAT "</Vf></Ei></F>";\
static const AZANDE_EVENT_INT NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	100,\
	__LINE__,\
	AZANDE_XML_STRING_##NAME,\
};\
static const AZANDE_EVENT_INT* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


#define define_int_event_id(NAME, TITLE, ID, SUBID, GROUP, UNIT, MIN, MAX, FORMAT)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><Ei><I>" #ID "</I><Is>" #SUBID "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><U>" UNIT "</U><Mn>" #MIN "</Mn><Mx>" #MAX "</Mx><Vf>" FORMAT "</Vf></Ei></F>";\
static const AZANDE_EVENT_INT NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	ID,\
	SUBID,\
	AZANDE_XML_STRING_##NAME,\
};\
static const AZANDE_EVENT_INT* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


// Double Event ------------------------------------------
#define define_double_event(NAME, TITLE, GROUP, UNIT, MIN, MAX, FORMAT)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><Ed><I>100</I><Is>" TOSTRING(__LINE__) "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><U>" UNIT "</U><Mn>" #MIN "</Mn><Mx>" #MAX "</Mx><Vf>" FORMAT "</Vf></Ed></F>";\
static const AZANDE_EVENT_DOUBLE NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	100,\
	__LINE__,\
	AZANDE_XML_STRING_##NAME,\
};\
static const AZANDE_EVENT_DOUBLE* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


#define define_double_event_id(NAME, TITLE, ID, SUBID, GROUP, UNIT, MIN, MAX, FORMAT)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><Ed><I>" #ID "</I><Is>" #SUBID "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><U>" UNIT "</U><Mn>" #MIN "</Mn><Mx>" #MAX "</Mx><Vf>" FORMAT "</Vf></Ed></F>";\
static const AZANDE_EVENT_DOUBLE NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	ID,\
	SUBID,\
	AZANDE_XML_STRING_##NAME,\
};\
static const AZANDE_EVENT_DOUBLE* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;


// Text Event ------------------------------------------
#define define_text_event(NAME, TITLE, GROUP, MAX_LEN)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><Et><I>100</I><Is>" TOSTRING(__LINE__) "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><Mx>" #MAX_LEN "</Mx></Et></F>";\
static const AZANDE_EVENT_TEXT NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	100,\
	__LINE__,\
	AZANDE_XML_STRING_##NAME,\
};\
static const AZANDE_EVENT_TEXT* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;



#define define_text_event_id(NAME, TITLE, ID, SUBID, GROUP, MAX_LEN)\
static const char AZANDE_XML_STRING_##NAME[] __FLASH_DIRECTIVE =  "<F><Et><I>" #ID "</I><Is>" #SUBID "</Is><N>" TITLE "</N><Gr>0" #GROUP "</Gr><Mx>" #MAX_LEN "</Mx></Et></F>";\
static const AZANDE_EVENT_TEXT NAME##_FEATURE_OBJ __FLASH_DIRECTIVE =\
{\
	ID,\
	SUBID,\
	AZANDE_XML_STRING_##NAME,\
};\
static const AZANDE_EVENT_TEXT* const NAME __FLASH_DIRECTIVE = & NAME##_FEATURE_OBJ;



#endif /* AZANDEUSERMACROS_H_ */
