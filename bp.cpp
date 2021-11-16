/* 046267 Computer Architecture - Spring 2020 - HW #1 */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include <stdbool.h>
#include <iostream>
#include <vector>
#include <memory>
#include <iostream>
#include <cmath>
#include <string>
#include <list>

#define SIZE_OF_BYTE 8
#define SIZE_OF_INT 32
#define VALID_BIT 1

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
	//history(const &history) = default;
	//history& operator=(const &history) = default;
	const int* getHistory ();
	void updateHistory (bool taken);
	const int historyArrToNum();
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

const int history::historyArrToNum()
{
	int historyAsNumber = 0;
	for (int i = 0; i < history_size; i++)
	{
		if (array[i])
		{
			historyAsNumber += pow(2,i);
		}
	}
	return historyAsNumber;
	
}

//opertor=

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
	fsm(int num_of_fsm, unsigned initial_state);
	~fsm();
	states* getCurrentState();
	void updateFSM (int fsm_num, bool taken);
};

fsm::fsm(int num_of_fsm, unsigned initial_state): fsm_num(num_of_fsm)
{
	FSMs = new states[fsm_num]();
	for (int i = 0 ; i < fsm_num ; i++){
		FSMs[i] = states(initial_state);
	}
}

fsm::~fsm()
{
	delete[] FSMs;
}

states* fsm::getCurrentState()
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
	std::shared_ptr<history> History;
	std::shared_ptr<fsm> FSM;

public:
	branch(uint32_t branchPC, uint32_t targetPC, std::shared_ptr<history> hist, std::shared_ptr<fsm> fsm, 
		   bool isGlobalHist, bool isGlobalFSM, int numOfHistBits, int numOfFSMBits, unsigned initial_state);
	~branch() = default;
	const uint32_t getBranchPC ();
	const uint32_t getTargetPC ();
	void updateTargetPC (const uint32_t new_PC);
	const std::shared_ptr<history> getHistory ();
	void updateHistory (bool taken);
	const std::shared_ptr<fsm> getFSM ();
	void updateFSM(int fsm_num, bool taken);
};

branch::branch(uint32_t branchPC, uint32_t targetPC, std::shared_ptr<history> hist, std::shared_ptr<fsm> Fsm,
			    bool isGlobalHist, bool isGlobalFSM, int numOfHistBits, int numOfFSMBits, unsigned initial_state):
			 		branchPC(branchPC), targetPC(targetPC){
	if (!isGlobalHist || hist == nullptr){
		History = std::make_shared<history>(new history(numOfHistBits));
	}
	else History = hist;
	if (!isGlobalFSM || Fsm == nullptr){
		FSM = std::make_shared<fsm>(new fsm(numOfFSMBits, initial_state)); 
	}
	else FSM = Fsm;
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

const std::shared_ptr<history> branch::getHistory()
{
	return History;
}

void branch::updateHistory(bool taken)
{
	History->updateHistory(taken);
}

const std::shared_ptr<fsm> branch::getFSM()
{
	return this->FSM;
}

void branch::updateFSM(int fsm_num, bool taken)
{
	FSM->updateFSM(fsm_num, taken);
}

/*
 Branch prediction class
 */
class bp
{
private:
	std::vector<branch*> btb_list;
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
	int calculteFsmPtr(uint32_t pc);

	friend int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
	int calculteFsmPtr();
	branch* createNewBranch(unsigned branchPC, unsigned targetPC, int taken);
	int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared);
	bool BP_predict(uint32_t pc, uint32_t *dst);
	void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst);
	void BP_GetStats(SIM_stats *curStats);

};

int calculteFsmPtr(uint32_t pc)
{
	int FsmRow = 0;
	uint32_t pc_in_list = this->btb_list[hash_func(this->btbSize, pc)].getBranchPC();
	int hist = this->btb_list[hash_func(this->btbSize, pc)].History.historyArrToNum();
	if (this->shared == 1)
	{
		int pcLSB = pc >> 2;
		int pcLSB = pcLSB & (pow(2, this->historySize) - 1);
		FsmRow = hist ^ pcLSB;
	}
	if (this->shared == 2)
	{
		int pcMB = pc >> 16;
		int pcMB = pcMB & (pow(2, this->historySize) - 1);
		FsmRow = hist ^ pcMB;
	}
	if (this->shared == 0)
	{
		FsmRow = hist;
		for (int i = 0; i < this->tagSize; i++)
		{
			FsmRow += pow(2,(pc_in_list >> (this->tagSize - i)));
		}
		
	}
	return FsmRow;
}

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
			btb_list = std::vector<branch*>(btbSize);
		}

int hash_func(unsigned btbSize, uint32_t pc){
	uint32_t temp = pc<<2;
	int index = int(pow(2, log2(btbSize))) & temp;
	return index;
}

int getTagBits(unsigned btbSize, uint32_t pc){
	uint32_t tag = pc<<int(2 + log2(btbSize));
	return tag;
}

branch* bp::createNewBranch(unsigned branchPC, unsigned targetPC, int taken){ //need to decide what to do with the bool taken. neccaserry?
	std::shared_ptr<history> hist = nullptr;
	std::shared_ptr<fsm> Fsm = nullptr;
	int fsm_size = int(pow(2,historySize));
	if(isGlobalHist){
		hist = std::make_shared<history>(new history(historySize));
	}
	if(isGlobalTable){
		states new_state = states(fsmState + taken) > ST? ST : states(fsmState + taken); 
		/* check the initial state of the fsm - if fsmState=ST and the branch was taken, the initial state will stay ST */
		Fsm = std::make_shared<fsm>(new fsm(fsm_size, states(new_state)));
	}
	return new branch(branchPC, targetPC, hist, Fsm, isGlobalHist, isGlobalTable, historySize, fsm_size, fsmState);
}

void bp::addNewBranch(uint32_t pc, uint32_t targetPc, bool taken)
{
	uint32_t pc_in_list =  btb_list[hash_func(btbSize, pc)]->getBranchPC();
	if(getTagBits(btbSize, pc_in_list) == getTagBits(btbSize, pc)){
		btb_list[pc_in_list]->updateTargetPC(targetPc); //****************not sure!!!! update the old targetPc to the new one? 
		btb_list[pc_in_list]->updateFSM(calculteFsmPtr(), taken); //* update the state machine of the existing pc branch
	}
	else btb_list[pc_in_list] = createNewBranch(pc, targetPc, int(taken));
}

bool bp::nextPred(uint32_t pc)
{
	states* Fsm = btb_list[hash_func(btbSize, pc)]->getFSM()->getCurrentState();
	if(Fsm[calculteFsmPtr()] == SNT || Fsm[calculteFsmPtr()] == WNT) return false;
	else return true;
}

int bp::calculateMemorySize()
{
	int fsm_size = int(pow(2, log2(historySize)));
	int hist_size = historySize;
	if(!isGlobalTable) fsm_size = fsm_size * btbSize;
	if(!isGlobalHist) hist_size = hist_size * historySize;
	return SIZE_OF_BYTE * (hist_size + fsm_size + SIZE_OF_INT + SIZE_OF_INT) + VALID_BIT;
}

void bp::statsUpdate(bool taken)
{
	if(!taken)
	{
		bp_stats.flush_num ++; //* update flush number only if necessary *//
	}
	bp_stats.br_num++;
	bp_stats.size = calculateMemorySize();
}

/**
 * hw functions
 * 
 **/
int bp::BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){
	if(IsdataValid(btbSize, historySize, tagSize, fsmState)) return -1;
	this->btb_list = std::vector<branch*>();
	this->btbSize = btbSize;
	this->historySize = historySize;
	this->tagSize = tagSize;
	this->fsmState = fsmState;
	this->isGlobalTable = isGlobalTable;
	this->Shared = Shared;
	this->isGlobalHist = isGlobalHist;
	return 0;
}

bool BP_predict(uint32_t pc, uint32_t *dst){
	if (!btb_list[hash_func(btbSize, pc)]) // pc is not branch operation
	{
		dst = pc + 4;
		return false
	}
	if (btb_list[hash_func(btbSize, pc)].FSM[calculteFsmPtr(pc)] == (SNT || WNT))
	{
		dst = pc + 4;
		return false;
	}
	dst = btb_list[hash_func(btbSize, pc)].getTargetPC();
	return true;
}

void bp::BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
	if(nextPred(pc)){  // check if the pc state machine shows to take the prediction or not
		addNewBranch(pc, pred_dst, taken); // add new noot to the btb - add the pred_dst because the prediction is taken
		this->statsUpdate(taken == nextPred(pc)); // change the SIM_stats saved in the predictor - add 1 to br_num, "false" = don't add any flush_num
	}
	else{
		addNewBranch(pc, targetPc, taken); // add new note to the BTB - add the targetPC because the prediction is not taken
		this->statsUpdate(taken == nextPred(pc)); // change the SIM_stats saved in the predictor - add 1 to br_num, "true" = add 3 to flush_num
	}
	return;
}

void bp::BP_GetStats(SIM_stats *curStats){
	curStats->br_num = bp_stats.br_num;
	curStats->flush_num = bp_stats.flush_num;
	curStats->size = bp_stats.size;
	delete(this);
}
