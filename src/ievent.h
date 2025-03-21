﻿#ifndef NS_EVENT_H_
#define NS_EVENT_H_

#include "glo_def.h"
#include "eventtype.h"
#include <string>

class iEvent
{
public:
	iEvent(u32 eid, u32 sn);

	virtual ~iEvent();

	virtual std::ostream& dump(std::ostream& out) const { return out; };
	virtual i32 Bytesize() { return 0; }
	virtual bool SerializeToArray(char* buf, int len) { return true; };

	u32 generateSeqNo();       //事件的序列号
	u32 get_eid() const { return eid_; };   //事件ID ，代表事件类型
	u32 get_sn() const { return sn_; };
	void* get_args() const { return args_; };

	void set_args(void* args) { args_ = args; };
	void set_eid(u32 eid) { eid_ = eid; };

private:
	u32 eid_;
	u32 sn_;
	void* args_;
};


#endif // !NS_EVENT_H_

