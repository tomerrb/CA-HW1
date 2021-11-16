/* 046267 Computer Architecture - Spring 2020 - HW #1 */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include <stdbool.h>
using std::shared_ptr
#include <vector>
#include <iostream>
#include <numeric>
#include <string>
#include <list>

enum states {SNT = 0, WNT = 1, WT = 2, ST = 3};

/**
 * history class
 * 
 **/
class history
{
private:
	int* array;
	int history_size;
public:
	history(int history_size);
	~history();
	const int* getHistory ();
	void updateHistory (bool taken);
};

history::history(int history_size)
{
	array = new int[history_size]();
	for (int i = 0 ; i < history_size ; i++){
		array[i] = 0;
	}
}
history::~history()
{
	delete[] array;
}
const int* history::getHistory()
{
	return this->array;
}
void history::updateHistory (bool taken)  // update the array - shift left
{
	for(int i = history_size - 1 ; i > 0 ; i--){
		array[i] = array[i-1];
	}
	if(taken) array[0] = 1;
	else array[0] = 0;
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
	fsm(int num_of_fsm);
	~fsm();
	const states* getFSM ();
	void updateFSM (int fsm_num, bool taken);
};

fsm::fsm(int num_of_fsm): fsm_num(num_of_fsm)
{
	FSMs = new states[fsm_num]();
	for (int i = 0 ; i < fsm_num ; i++){
		FSMs[i] = WNT;
	}
}

fsm::~fsm()
{
	delete[] FSMs;
}

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


/**
 * branch class
 * 
 **/
class branch
{
private:
	uint32_t branchPC;
	uint32_t targetPC;
	history History;
	fsm FSM;

public:
	branch(uint32_t branchPC, uint32_t targetPC, shared_ptr<history> hist, shared_ptr<fsm> fsm, bool isGlobalHist, bool isGlobalFSM, int numOfHistBits, int numOfFSMBits);
	~branch() = default;
	const uint32_t getBranchPC ();
	const uint32_t getTargetPC ();
	void updateTargetPC (const uint32_t new_PC);
	const history getHistory ();
	void updateHistory (bool taken);
	const fsm getFSM ();
	void updateFSM(bool taken);
};

branch::branch(uint32_t branchPC, uint32_t targetPC, shared_ptr<history> hist, shared_ptr<fsm> fsm, bool isGlobalHist, bool isGlobalFSM, int numOfHistBits, int numOfFSMBits): branchPC(branchPC), targetPC(targetPC)
{
	if (!isGlobalHist || hist == nullptr)
	{
		history h =new history(numOfHistBits);
		if (isGlobalHist)
		{
			History = make_shared<history>(h);
		}
		else {
			History = h;
		}
	}
	else
	{
		History = hist;
	}
	if (!isGlobalFSM || fsm == nullptr)
	{
		fsm f = new fsm(numOfFSMBits);
		if (isGlobalFSM)
		{
			FSM = make_shared<fsm>(f);
		}
		else
		{
			FSM = f;
		}
	}
	else
	{
		FSM = fsm;
	}
} 

const uint32_t branch::getBranchPC()
{
	return this->branchPC;
}

const uint32_t branch::getTargetPC()
{
	return this->targetPC;
}

void branch::updateTargetPC(const uint32_t new_PC)
{
	this->targetPC = new_PC;
}

const history branch::getHistory()
{
	return this->History;
}

void branch::updateHistory(bool taken)
{
	this->History.updateHistories(taken);
}

const fsm branch::getFSM()
{
	return this->FSM;
}

void branch::updateFSM(bool taken)
{
	FSM.updateFSM(taken);
}

/*
 Branch prediction class
 */
class bp
{
private:
	std::vector<branch> btb_list;
	unsigned btbSize;
	unsigned historySize;
	unsigned tagSize;
	unsigned fsmState;
	bool isGlobalHist;
	bool isGlobalTable;
	int Shared;
	SIM_stats bp_stats;

public:
	bp(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned FSMState, bool isGlobalHist, bool isGlobalTable, int shared);
	~bp() = default;
	void addNewBranch(uint32_t pc, uint32_t targetPc, bool taken);
	bool nextPred(uint32_t pc);
	int calculateMemorySize();
	void statsUpdate(bool taken);
	int calculteFsmPtr();

	friend int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared);
	friend bool BP_predict(uint32_t pc, uint32_t *dst);
	friend void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst);
	friend void BP_GetStats(SIM_stats *curStats);

};

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

bp::bp(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned FSMState, bool isGlobalHist, bool isGlobalTable, int shared):
		btbSize(btbSize), historySize(historySize), tagSize(tagSize), fsmState(FSMState),
		isGlobalHist(isGlobalHist), isGlobalTable(isGlobalTable), Shared(shared){
			btb_list = std::vector<branch>(btbSize);
		}

void bp::addNewBranch(uint32_t pc, uint32_t targetPc, bool taken)
{
	branch iterator = btb_list.begin;
	std::for_each(btb_list.begin, btb_list.end, [](branch &iterator){
		if (iterator.getBranchPC() == pc)		{
			iterator.updateTargetPC(targetPc); //****************not sure!!!! update the old targetPc to the new one? 
			iterator.updateFSM(taken); //* update the state machine of the existing pc branch
			return;
		}
	})
	btb_list.push_back(branch(pc, targetPc));
}

bool bp::nextPred(uint32_t pc)
{
	for (int i = 0 ; i < btb_size ; i++)
	{
		if (btb_list[i].branchPc == pc)
		{
			if(btb_list[i].FSM.fsm_state == SNT || btb_list[i].FSM.fsm_state == WNT) return false;
			else return true;
		}
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
	curStats->br_num = this->btb->getBTBNum();
	curStats->flush_num = this->getFlushNum();
	curStats->size = this->calculateMemorySize();
	delete(something!!!!!!!!!);
}
	return;
}

