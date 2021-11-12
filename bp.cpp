/* 046267 Computer Architecture - Spring 2020 - HW #1 */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){
	return -1;
}

bool BP_predict(uint32_t pc, uint32_t *dst){
	return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
	this->addNewNoteToBTB(pc, targetPc, taken); // add new note to the BTB 
	if(this->nextPred(taken)){  // changes the SM and returns if the prediction is taken or not
		this->changePc(pc,pred_dst); //change the current pc of the predictor
		this->statsUpdate(false); // change the SIM_stats saved in the predictor - add 1 to br_num, "false" = don't add any flush_num
	}
	else{
		this->changePc(pc,targetPc);
		this->statsUpdate(true); // change the SIM_stats saved in the predictor - add 1 to br_num, "true" = add 3 to flush_num
	}
	return;
}

void BP_GetStats(SIM_stats *curStats){
	curStats->br_num = this->getBrNum();
	curStats->flush_num = this->getFlushNum();
	curStats->size = this->calculateSize();
	delete(this);
}
