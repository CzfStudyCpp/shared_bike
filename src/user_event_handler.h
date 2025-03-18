#ifndef BRKS_BUS_USERM_HANDLER_H_
#define BRKS_BUS_USERM_HANDLER_H_

#include "glo_def.h"
#include "iEventHandler.h"
#include "events_def.h"
#include "threadpool/thread.h"

#include <map>

class UserEventHandler : public iEventHandler
{
public:
	UserEventHandler();
	virtual ~UserEventHandler();
	virtual iEvent* handle(const iEvent* ev);

private:
	
	MobileCodeRspEv* handle_mobile_code_req(MobileCodeReqEv* ev);
	RegisterResEv* handle_user_register_req(RegisterReqEv* ev);
	AddBikeResEv* handle_addBike_req(AddBikeReqEv* ev);
	DeleteBikeResEv* handle_deleteBike_req(DeleteBikeReqEv* ev);
	i32 code_gen();
	LoginResEv* handle_login_req(LoginReqEv* ev);
	bikeScanQRStartResponseEv* handle_bike_scanQRSatrt_req(bikeScanQRStartReqEv* ev);
	bikeScanQREndResponseEv* handle_bike_scanQREnd_req(bikeScanQREndReqEv* ev);
	bikeScanQRFaultResponseEv* handle_bike_scanQRFault_req(bikeScanQRFaultReqEv* ev);
	bikeScanQRRepairFinishResponseEv* handle_bike_scanQRRepairFinish_req(bikeScanQRRepairFinishReqEv* ev);
	broadcastMsgResponseEv* handle_broadcast_msg_req(broadcastMsgReqEv* ev);
	bikeInfoResponseEv* handle_bikeInfo_req(bikeInfoReqEv* ev);

	ListAccountRecordsResonseEv* handle_list_account_records_req(ListAccountRecordsReqEv* ev);
	
private:
	//std::string mobile_;
	std::map<std::string, i32> m2c_;  //first is mobile, second is code
	pthread_mutex_t pm_;
};

#endif // !BRKS_BUS_USERM_HANDLER_H_

