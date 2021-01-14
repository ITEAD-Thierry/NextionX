#define initComm(n, o)                     \
	_NexCom<decltype(n) &> o##Comm(n, #o); \
	typedef _NexComp<decltype(o##Comm) &, o##Comm> o##Comp
#define initCommDbg(n, d, o)                                    \
	_NexComDbg<decltype(n) &, decltype(d) &> o##Comm(n, d, #o); \
	typedef _NexComp<decltype(o##Comm) &, o##Comm> o##Comp

template <class nexSerType>
class _NexCom
{
public:
	_NexCom(nexSerType &nexSer, const char *tag) : _nexSer(nexSer), _tag(tag) {}
	void begin(uint32_t nexBaud = 9600)
	{
		this->_nexSer.begin(nexBaud);
		while (!this->_nexSer)
			;
		this->cmdWrite("");
	}
	void cmdWrite(String cmd)
	{
		this->_nexSer.print(cmd);
		this->_nexSer.write("\xFF\xFF\xFF");
	}

protected:
	const char *_tag;
private:
	nexSerType &_nexSer;
};

template <class nexSerType, class dbgSerType>
class _NexComDbg : protected _NexCom<nexSerType>
{
public:
	_NexComDbg(nexSerType &nexSer, dbgSerType &dbgSer, const char *tag) : _NexCom<nexSerType>::_NexCom(nexSer, tag), _dbgSer(dbgSer), _dbgEn(true) {}
	void begin(uint32_t nexBaud = 9600, uint32_t dbgBaud = 9600)
	{
		this->_dbgSer.begin(dbgBaud);
		while (!this->_dbgSer)
			;
		this->dbgWrite("Dbg ready");
		_NexCom<nexSerType>::begin(nexBaud);
		this->dbgWrite("Com ready");
	}
	void dbgEnable(bool en)
	{
		this->_dbgEn = en;
	}
	void cmdWrite(String cmd)
	{
		_NexCom<nexSerType>::cmdWrite(cmd);
		this->dbgWrite((String) "Cmd sent: " + cmd);
	}

private:
	void dbgWrite(String txt)
	{
		if (_dbgEn)
		{
			this->_dbgSer.println((String)this->_tag + "\t" + txt);
		}
	}
	dbgSerType &_dbgSer;
	bool _dbgEn;
};

template <class nexCommType, nexCommType nexComm>
class _NexComp
{
public:
	_NexComp(uint8_t pageId, uint8_t objId = 0) : _page(pageId), _obj(objId) {}
	void setAttr(const char *attr, int32_t num)
	{
		this->_nexComm.cmdWrite((String) "p[" + this->_page + "].b[" + this->_obj + "]." + attr + "=" + num);
	}
	void setAttr(const char *attr, String txt)
	{
		this->_nexComm.cmdWrite((String) "p[" + this->_page + "].b[" + this->_obj + "]." + attr + "=\"" + txt + "\"");
	}

private:
	static constexpr nexCommType &_nexComm = nexComm;
	uint8_t _page;
	uint8_t _obj;
};