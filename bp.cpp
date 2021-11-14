/* 046267 Computer Architecture - Spring 2020 - HW #1 */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include <stdbool.h>

enum states {SNT = 0, WNT = 1, WT = 2, ST = 3};

/**
 * history class
 * 
 **/
class history
{
private:
	int* histories;
	int histories_num;
public:
	history(int* histories, int histories_num);
	~history() = default;
	const int* getHistories ();
	void updateHistories (int* new_hist);
	const int getNumOfHistories ();
	void updateHistoriesNum (int new_num);

};

history::history(int* histories, int histories_num) : histories(histories), histories_num(histories_num)
{
}

const int* history::getHistories()
{
	return this->histories;
}

void history::updateHistories (int* new_hist)
{
	this->histories = new_hist;
}

const int history::getNumOfHistories ()
{
	return this->histories_num;
}

void history::updateHistoriesNum (int new_num)
{
	this->histories_num = new_num;
}

/**
 * fsm class
 * 
 **/
class fsm
{
private:
	states* FSMs;
	int fsm_num;

public:
	fsm(states* fsms, int num_of_fsm);
	~fsm()= default;
	const states* getFSM ();
	void updateFSM (states* new_fsm);
	const int getNumOfFSM ();
	void updateFSMNum (int new_num);
};

fsm::fsm(states* fsms, int num_of_fsm) : FSMs(fsms), fsm_num(num_of_fsm)
{
}

const states* fsm::getFSM()
{
	return this->FSMs;
}

void fsm::updateFSM(states* new_fsm)
{
	this->FSMs = new_fsm;
}

const int fsm::getNumOfFSM()
{
	return this->fsm_num;
}

void fsm::updateFSMNum(int new_num)
{
	this->fsm_num = new_num;
}


/**
 * branch class
 * 
 **/
class branch
{
private:
	uint32_t branchPC;
	uint32_t targetPC;
	history* History;
	bool prediction;
	fsm* FSM;

public:
	branch(uint32_t branchPC, uint32_t targetPC, history* hist, bool pred, fsm* fsm);
	~branch() = default;
	const uint32_t getBranchPC ();
	void updateBranchPC (const uint32_t new_PC);
	const uint32_t getTargetPC ();
	void updateTargetPC (const uint32_t new_PC);
	const history* getHistory ();
	void updateHistory (history* new_history);
	const bool getPred ();
	void updatePred (const bool new_pred);
	const fsm* getFSM ();
	void updateFSM(fsm* new_FSM);
};

branch::branch(uint32_t branchPC, uint32_t targetPC, history* hist, bool pred,  fsm* fsm) : branchPC(branchPC),
				targetPC(targetPC), History(hist), prediction(pred), FSM(fsm)
{
}

const uint32_t branch::getBranchPC()
{
	return this->branchPC;
}

void branch::updateBranchPC(const uint32_t new_PC)
{
	this->branchPC = new_PC;
}

const uint32_t branch::getTargetPC()
{
	return this->targetPC;
}

void branch::updateTargetPC(const uint32_t new_PC)
{
	this->targetPC = new_PC;
}

const history* branch::getHistory()
{
	return this->History;
}

void branch::updateHistory(history* new_history)
{
	this->History = new_history;
}

const bool branch::getPred()
{
	return this->prediction;
}

void branch::updatePred(const bool new_pred)
{
	this->prediction = new_pred;
}

const fsm* branch::getFSM()
{
	return this->FSM;
}

void branch::updateFSM(fsm* new_fsm)
{
	this->FSM = new_fsm;
}



/**
 * btb class
 * 
 **/
class btb
{
private:
	branch* branch_arr;
	int branches_Num;

public:
	btb(branch* b, int branch_num);
	~btb() = default;
	const branch* getBranches();
	void updateBranch(branch* new_branch_arr);
	const int getBranchesNum();
	void updateBranchesNum(const int new_num);
};

btb::btb(branch* b, int branch_num): branch_arr(b), branches_Num(branch_num)
{
}

const branch* btb::getBranches()
{
	return this->branch_arr;
}

void btb::updateBranch(branch* new_branch_arr)
{
	this->branch_arr =new_branch_arr; 
}

const int btb::getBranchesNum()
{
	return this->branches_Num;
}

void btb::updateBranchesNum(const int new_num)
{
	this->branches_Num = new_num;
}

/**
 * bp class
 * 
 **/
class bp
{
private:
	btb* btb;
	unsigned btbSize;
	unsigned historySize;
	unsigned tagSize;
	unsigned fsmState;
	bool isGlobalHist;
	bool isGlobalTable;
	int Shared;

public:
	bp(btb* BTB, unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned FSMState, bool isGlobalHist, bool isGlobalTable, int shared);
	~bp() = default;
};

bp::bp(btb* BTB, unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned FSMState, bool isGlobalHist, bool isGlobalTable, int shared):
		btb(BTB), btbSize(btbSize), historySize(historySize), tagSize(tagSize), fsmState(FSMState), isGlobalHist(isGlobalHist), isGlobalTable(isGlobalTable), shared(shared)
{
}


/**
 * hw functions
 * 
 **/
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
	return;
}

