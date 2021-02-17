#ifndef NextionX_h
#define NextionX_h

#define MAX_IN_BUF_LEN 14
#define MAX_LS_LST_LEN 16

/*
 * CompId declaration
 */

typedef union CompId_t
{
	uint16_t guid;
	struct
	{
		uint8_t page;
		uint8_t obj;
	};
} CompId;

/*
 * Forward declaration (needed for the reference in NexComp)
 */
class NexComm;

/* 
 * NexComp declaration
 */
class NexComp
{
public:
	template <class NexComm_t>
	NexComp(NexComm_t &, uint8_t, uint8_t);
	void setAttr(const char *, int32_t);
	void setAttr(const char *, String);
	uint16_t getGuid();
	void setOnTouch(void (*)());
	void setOnRelease(void (*)());
	void callBack(uint8_t);

private:
	NexComm *_nexComm;
	CompId _myId;
	void (*_onPrs)() = nullptr;
	void (*_onRel)() = nullptr;
};

/*
 * ListEl declaration
 */
typedef struct ListEl_t
{
	uint16_t guid;
	NexComp *comp;
} ListEl;

/* 
 * NexComm declaration
 */
class NexComm
{
public:
	NexComm();
	template <class nexSerType>
	void begin(nexSerType &, uint32_t);
	template <class nexSerType>
	void begin(nexSerType &);
	template <class dbgSerType>
	void addDebug(dbgSerType &, uint32_t);
	template <class dbgSerType>
	void addDebug(dbgSerType &);
	void cmdWrite(String);
	void loop();

protected:
	void _addCmpList(NexComp *);
	uint8_t _inStr[MAX_IN_BUF_LEN] = {0};
	uint8_t _rLen = 0;
	Stream *_nexSer = nullptr;
	Stream *_dbgSer = nullptr;
	char *_tag;

private:
	void _nexInit();
	void _dbgInit();
	uint8_t _readNextRtn();
	uint8_t _fIdxByGuid(uint16_t);
	void _dbgLoop();
	uint8_t _inPtr = 0;
	uint8_t _ffCnt = 0;
	uint8_t _lsPtr = 0;
	ListEl _lsLst[MAX_LS_LST_LEN];

friend NexComp;
};

/*
 * NexComp Implementation
 */
template <class NexComm_t>
NexComp::NexComp(NexComm_t &nexComm, uint8_t pageId, uint8_t objId) : _nexComm(&nexComm)
{
	this->_myId.page = pageId;
	this->_myId.obj = objId;
}
void NexComp::setAttr(const char *attr, int32_t num)
{
	this->_nexComm->cmdWrite((String) "p[" + this->_myId.page + "].b[" + this->_myId.obj + "]." + attr + "=" + num);
}
void NexComp::setAttr(const char *attr, String txt)
{
	this->_nexComm->cmdWrite((String)"p[" + this->_myId.page + "].b[" + this->_myId.obj + "]." + attr + "=\"" + txt + "\"");
}
uint16_t NexComp::getGuid()
{
	return this->_myId.guid;
}
void NexComp::setOnTouch(void (*onPress)() = nullptr)
{
	this->_onPrs = onPress;
	this->_nexComm->_addCmpList(this);
}
void NexComp::setOnRelease(void (*onRelease)() = nullptr)
{
	this->_onRel = onRelease;
	this->_nexComm->_addCmpList(this);
}
void NexComp::callBack(uint8_t evt)
{
	switch (evt)
	{
	case 0:
		if (this->_onRel != nullptr)
		{
			this->_onRel();
		}
		break;
	case 1:
		if (this->_onPrs != nullptr)
		{
			(this->_onPrs());
		}
		break;
	}
}

/* 
 * NexComm implementation:
 */
NexComm::NexComm() {}
template <class nexSerType>
void NexComm::begin(nexSerType &nexSer, uint32_t nexBaud)
{
	nexSer.begin(nexBaud);
	while (!nexSer)
		;
	this->_nexSer = &nexSer;
	this->cmdWrite("");
	this->cmdWrite("bkcmd=0");
}
template <class nexSerType>
void NexComm::begin(nexSerType &nexSer)
{
	nexSer.begin(9600);
	while (!nexSer)
		;
	this->_nexSer = &nexSer;
	this->cmdWrite("");
	this->cmdWrite("bkcmd=0");
}
template <class dbgSerType>
void NexComm::addDebug(dbgSerType &dbgSer, uint32_t dbgBaud)
{
	dbgSer.begin(dbgBaud);
	while (!dbgSer)
		;
	this->_dbgSer = &dbgSer;
	this->cmdWrite("bkcmd=3");
}
template <class dbgSerType>
void NexComm::addDebug(dbgSerType &dbgSer)
{
	dbgSer.begin(9600);
	while (!dbgSer)
		;
	this->_dbgSer = &dbgSer;
	this->cmdWrite("bkcmd=3");
}
void NexComm::cmdWrite(String cmd)
{
	this->loop(); //handle incoming stuff first
	this->_nexSer->print(cmd); 
	this->_nexSer->print("\xFF\xFF\xFF");
	if ((this->_dbgSer != nullptr) && (cmd.length() > 0))
	{
		this->_dbgSer->println(cmd);
	}
}
void NexComm::loop()
{
	CompId trCmp;
	this->_rLen = this->_readNextRtn();
	if ((this->_rLen > 0) && (this->_dbgSer != nullptr))
	{
		this->_dbgLoop();
	}
	if ((this->_rLen == 4) && (this->_inStr[0] == 0x65))
	{
		trCmp.page = this->_inStr[1];
		trCmp.obj = this->_inStr[2];
		uint8_t listpos = this->_fIdxByGuid(trCmp.guid);
		if (listpos < MAX_LS_LST_LEN)
		{
			this->_lsLst[listpos].comp->callBack(this->_inStr[3]);
		}
	}
}
void NexComm::_addCmpList(NexComp *cmp)
{
	ListEl mycmp;
	mycmp.comp = cmp;
	mycmp.guid = cmp->getGuid();
	if (this->_fIdxByGuid(mycmp.guid) == 0xFF)
	{
		this->_lsLst[this->_lsPtr++] = mycmp;
		if (this->_lsPtr >= MAX_LS_LST_LEN)
		{
			this->_lsPtr = 0;
		}
	}
}
void NexComm::_dbgLoop()
{
	String retStr;
	if ((this->_rLen == 1) && ((this->_inStr[0] < 0x25) || (this->_inStr[0] > 0x85)))
	{
		if (this->_inStr[0] == 1)
		{
			this->_dbgSer->println("Success");
		}
		else if (this->_inStr[0] < 0x25)
		{
			this->_dbgSer->println("Error " + String(this->_inStr[0], HEX));
		}
		else if (this->_inStr[0] > 0x85)
		{
			this->_dbgSer->println("Status " + String(this->_inStr[0], HEX));
		}
	}
	else if ((this->_rLen == 4) && (this->_inStr[0] == 0x65))
	{

		this->_dbgSer->println((String) (this->_inStr[3] == 1 ? "Press" : "Release") + " page " + this->_inStr[1] + " obj " + this->_inStr[2]);
	}
}
uint8_t NexComm::_readNextRtn()
{
	uint8_t msgLen = 0;
	while (this->_nexSer->available() && this->_ffCnt < 3)
	{
		uint8_t _inByte = this->_nexSer->read();
		this->_inStr[this->_inPtr++] = _inByte;
		if (_inByte == 255)
		{
			this->_ffCnt++;
		}
		else
		{
			this->_ffCnt = 0;
			if (this->_inPtr > (MAX_IN_BUF_LEN - 3))
			{
				this->_inPtr = MAX_IN_BUF_LEN - 3;
			}
		}
	}
	if (this->_ffCnt == 3)
	{
		msgLen = this->_inPtr - 3;
		this->_ffCnt = 0;
		this->_inPtr = 0;
	}
	return msgLen;
}
uint8_t NexComm::_fIdxByGuid(uint16_t sguid)
{
	uint8_t retval = 0xFF;
	for (uint8_t i = 0; i < MAX_LS_LST_LEN; i++)
	{
		if (this->_lsLst[i].guid == sguid)
		{
			retval = i;
			break;
		}
	}
	return retval;
}

#endif