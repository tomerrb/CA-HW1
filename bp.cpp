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

history::history(int* histories, int histories_num) : histories(histories), histories_num(histories_num) // build a std::array
{
}

const int* history::getHistories()
{
	return this->histories;
}

void history::updateHistories (int* new_hist)  // update the array - shift left
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
	void updateFSM (bool taken);
	const int getNumOfFSM ();
};

fsm::fsm(states* fsms, int num_of_fsm) : FSMs(fsms), fsm_num(num_of_fsm){} // initial array to WNT in each cell

const states* fsm::getFSM()
{
	return this->FSMs;
}

void fsm::updateFSM(int fsm_num, bool taken)
{
	if(FSMs[fsm_num] == SNT)
	{
		if(taken) FSMs[fsm_num] = WNT;
	}
	if (FSMs[fsm_num] == WNT)
	{
		if(taken) FSMs[fsm_num] = WT;
		else FSMs[fsm_num] = SNT;
	}
	
	if (FSMs[fsm_num] == WT)
	{
		if(taken) FSMs[fsm_num] = ST;
		else FSMs[fsm_num] = WNT;
	}
	
	if (FSMs[fsm_num] == ST)
	{
		if(!taken) FSMs[fsm_num] = WT;
	}
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
	fsm* FSM;

public:
	branch(uint32_t branchPC, uint32_t targetPC, history* hist, fsm* fsm);
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
	void updateFSM(bool taken);
};

branch::branch(uint32_t branchPC, uint32_t targetPC, history* hist, bool pred,  fsm* fsm)
{

} 
: branchPC(branchPC),
				targetPC(targetPC), History(hist), prediction(pred), FSM(fsm) //* update the ctor to two possiblities - global FSM and private
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

void branch::updateFSM(bool taken)
{
	FSM->updateFSM(taken);
}


/**
 * btb class
 * 
 **/
class btb
{
private:
	branch* branch_arr; // update to build a list
	unsigned int branches_Num;
public:
	btb(branch* b, int branch_num); // add global/local + shared
	~btb() = default;
	const branch* getBranches();
	void updateBranch(branch* new_branch_arr);
	const int getBranchesNum();
	void updateBranchesNum(const int new_num);
	void addNewBranch(uint32_t pc, uint32_t targetPc, bool taken);
	bool nextPred(uint32_t pc);

};

btb::btb(branch* b, int branch_num): branch_arr(b), branches_Num(branch_num){} // update to build a list

const branch* btb::getBranches()
{
	return this->branch_arr;
}

void btb::updateBranch(branch* new_branch_arr)
{
	this->branch_arr = new_branch_arr; 
}

const int btb::getBranchesNum()
{
	return this->branches_Num;
}

void btb::updateBranchesNum(const int new_num)
{
	this->branches_Num = new_num;
}

void btb::addNewBranch(uint32_t pc, uint32_t targetPc, bool taken)
{
	for(int i = 0 ; i < branches_Num ; i++)
	{
		if (branch_arr[i].branchPc == pc)
		{
			branch_arr[i].targetPC = targetPc; //****************not sure!!!! update the old targetPc to the new one? 
			branch_arr[i].updateFSM(taken); //* update the state machine of the existing pc branch
		}
	}
	if(not enough space in the array) allocate more;
	branch_arr[branches_Num] = new branch(pc, targetPc);
	branches_Num++;

}

bool btb::nextPred(uint32_t pc)
{
	for (int i = 0 ; i < branches_Num ; i++)
	{
		if (branch_arr[i].branchPc == pc)
		{
			if(branch_arr[i].FSM.fsm_state == SNT || branch_arr[i].FSM.fsm_state == WNT) return false;
			else return true;
		}
	}
	return false;
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
	SIM_stats bp_stats;

public:
	bp(btb* BTB, unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned FSMState, bool isGlobalHist, bool isGlobalTable, int shared);
	~bp() = default;
	friend int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared);
	friend bool BP_predict(uint32_t pc, uint32_t *dst);
	friend void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst);
	friend void BP_GetStats(SIM_stats *curStats);
	int calculateMemorySize();
	void statsUpdate(bool taken);
};

bp::bp(btb* BTB, unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned FSMState, bool isGlobalHist, bool isGlobalTable, int shared):
		btb(BTB), btbSize(btbSize), historySize(historySize), tagSize(tagSize), fsmState(FSMState),
		isGlobalHist(isGlobalHist), isGlobalTable(isGlobalTable), shared(shared){}

bool IsdataValid(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState)
	{
		if(historySize < 1 || historySize > 8) return false;

		if(tagSize > 30 - btbSize || tagSize < 0) return false;

		if(fsmState > 3) return false;

		for(int i = 0 ; i < 6 ; i++)
		{
			if (btbSize == 2^i) return true;
		}

		return false;
	}


void bp::statsUpdate(bool taken)
{
	if(!taken)
	{
		bp_stats.flush_num += 3; //* update flush number only if necessary *//
	}
	bp_stats.br_num++;
	bp_stats.size = some number we need to update....;
}

/**
 * hw functions
 * 
 **/
int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){
	if(IsdataValid(btbSize, historySize, tagSize, fsmState, isGlobalHist, isGlobalTable, Shared)) return -1;
	
	this->btb = new btb;
	this->btbSize = btbSize;
	this->historySize = historySize;
	this->tagSize = tagSize;
	this->fsmState = fsmState;
	this->isGlobalHist = isGlobalHist;
	this->isGlobalTable = isGlobalTable;
	this->Shared = Shared;
	return 0;
}

bool BP_predict(uint32_t pc, uint32_t *dst){
	return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
	if(this->btb->nextPred(pc)){  // check if the pc state machine shows to take the prediction or not
		this->btb->addNewBranch(pc, pred_dst, taken); // add new noot to thr btb - add the pred_dst because the prediction is taken
		this->statsUpdate(false); // change the SIM_stats saved in the predictor - add 1 to br_num, "false" = don't add any flush_num
	}
	else{
		this->btb->addNewBranch(pc, targetPc, taken); // add new note to the BTB - add the targetPC because the prediction is not taken
		this->statsUpdate(true); // change the SIM_stats saved in the predictor - add 1 to br_num, "true" = add 3 to flush_num
	}
	return;
}

void BP_GetStats(SIM_stats *curStats){
	curStats->br_num = this->btb->getBranchesNum();
	curStats->flush_num = this->getFlushNum();
	curStats->size = this->calculateMemorySize();
	delete(something!!!!!!!!!);
}
	return;
}

