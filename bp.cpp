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
#include <algorithm>

#define SIZE_OF_BYTE 8
#define SIZE_OF_PC 32
#define SIZE_OF_TARGET_PC 30
#define VALID_BIT 1

enum states {SNT = 0, WNT = 1, WT = 2, ST = 3};

int history_size = 0;
int fsm_size = 0;

int calculateFsmSize(unsigned historySize){
	return int(pow(2,historySize));
}


/**
 * history class
 * 
 **/
class history
{
private:
	bool* array;
public:
	history();
	~history();
	history(const history& hist);
	history& operator=(const history& hist);
	const bool* getHistory();
	int getSize();
	void updateHistory (bool taken);
	const int historyArrToNum();
	friend class bp;
};

history::history()
{
	array = new bool[history_size]();
	for (int i = 0 ; i < history_size ; i++){
		array[i] = 0;
	}
}

history::~history()
{
	delete[] array;
}

const bool* history::getHistory()
{
	return this->array;
}

int history::getSize(){
	return history_size;
}

history::history(const history& hist){
	for(int i = 0 ; i < history_size ; i++){
		array[i] = hist.array[i];
	}
}

history& history::operator=(const history& hist){
	if (this == &hist){
		return *this;
	}
	delete[] array;
	array = new bool[history_size];
	for(int i = 0 ; i < history_size ; i++){
		array[i] = hist.array[i];
	}
	return *this;
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
	fsm& operator=(const fsm& Fsm);
	friend class bp;
};

fsm::fsm(int num_of_fsm = fsm_size, unsigned initial_state = WNT): fsm_num(num_of_fsm)
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

fsm& fsm::operator=(const fsm& Fsm){
	if (this == &Fsm){
		return *this;
	}
	delete[] FSMs;
	FSMs = new states[Fsm.fsm_num];
	for(int i = 0 ; i < Fsm.fsm_num ; i++){
		FSMs[i] = Fsm.FSMs[i];
	}
	fsm_num = Fsm.fsm_num;
	return *this;
}

states* fsm::getCurrentState()
{
	return this->FSMs;
}

void fsm::updateFSM(int fsm_num, bool taken)
{
	if(FSMs[fsm_num] == SNT){
		if(taken) FSMs[fsm_num] = WNT;
	}
	else if (FSMs[fsm_num] == WNT){
		if(taken) FSMs[fsm_num] = WT;
		else FSMs[fsm_num] = SNT;
	}
	else if (FSMs[fsm_num] == WT){
		if(taken) FSMs[fsm_num] = ST;
		else FSMs[fsm_num] = WNT;
	}
	else if (FSMs[fsm_num] == ST){
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
	history local_hist;
	fsm local_fsm;

public:
	branch(uint32_t branchPC, uint32_t targetPC, bool isGlobalHist, bool isGlobalFSM,
		   int numOfHistBits, int numOfFSMBits, unsigned initial_state);
	branch(const branch& to_copy);
	~branch() = default;
	const uint32_t getBranchPC();
	const uint32_t getTargetPC();
	void updateTargetPC (const uint32_t new_PC);
	const history& getHistory();
	void updateHistory (bool taken);
	const fsm& getFSM();
	void updateFSM(int fsm_num, bool taken);
	branch& operator=(const branch& to_copy);
	friend class bp;
};

branch::branch(uint32_t branchPC = 0, uint32_t targetPC = 0, bool isGlobalHist = false, bool isGlobalFSM = false,
			   int numOfHistBits = history_size, int numOfFSMBits = fsm_size, unsigned initial_state = WNT):
			 		branchPC(branchPC), targetPC(targetPC){
	if (!isGlobalHist){
		local_hist = history();
	}
	if (!isGlobalFSM){
		local_fsm = fsm(numOfFSMBits, initial_state); 
	}
}

branch::branch(const branch& to_copy): branchPC(to_copy.branchPC), targetPC(to_copy.targetPC), 
									   local_hist(to_copy.local_hist), local_fsm(to_copy.local_fsm){}

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

const history& branch::getHistory()
{
	return local_hist;
}

void branch::updateHistory(bool taken)
{
	local_hist.updateHistory(taken);
}

const fsm& branch::getFSM()
{
	return local_fsm;
}

void branch::updateFSM(int fsm_num, bool taken)
{
	local_fsm.updateFSM(fsm_num, taken);
}
branch& branch::operator=(const branch& to_copy)
{
	if (this == &to_copy)
	{
		return *this;
	}
	this->branchPC = to_copy.branchPC;
	this->targetPC = to_copy.targetPC;
	this->local_hist = to_copy.local_hist;
	this->local_fsm = to_copy.local_fsm;
	return *this;
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
	history global_history;
	fsm global_fsm;

public:
	bp(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned FSMState, bool isGlobalHist, bool isGlobalTable, int shared);
	~bp() = default;
	void addNewBranch(uint32_t pc, uint32_t targetPc, bool taken);
	bool nextPred(uint32_t pc);
	int calculateMemorySize(); 
	void statsUpdate(bool taken);
	int calculateFsmPtr(uint32_t pc);
	const branch& createNewBranch(unsigned branchPC, unsigned targetPC, bool taken);
	void BP_init_update(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned FSMState, bool isGlobalHist, bool isGlobalTable, int shared);
	friend bool BP_predict(uint32_t pc, uint32_t *dst);
	friend void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst);
	friend void BP_GetStats(SIM_stats *curStats);
	
};

bp main_bp = bp(0,0,0,0,false,false,0); //Global branch predictor

int hash_func(unsigned btbSize, uint32_t pc){
	uint32_t temp = pc>>2;
	int index = int(pow(2, log2(btbSize) - 1)) & temp;
	return index;
}

int bp::calculateFsmPtr(uint32_t pc)
{
	int FsmRow = 0;
	int bhr_index = hash_func(this->btbSize, pc);
	uint32_t pc_in_list = this->btb_list[bhr_index].getBranchPC();
	int hist;
	if(isGlobalHist){
		hist = main_bp.global_history.historyArrToNum();
	}
	else hist = this->btb_list[bhr_index].local_hist.historyArrToNum();

	if (this->Shared == 1)
	{
		int pcLSB = pc >> 2;
		pcLSB = pcLSB & int(pow(2, history_size) - 1);
		FsmRow = hist ^ pcLSB;
	}
	if (this->Shared == 2)
	{
		int pcMB = pc >> 16;
		pcMB = pcMB & int(pow(2, history_size) - 1);
		FsmRow = hist ^ pcMB;
	}
	if (this->Shared == 0)
	{
		FsmRow = hist;		
	}
	return FsmRow;
}

bool IsdataValid(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState)
	{
		if(historySize < 1 || historySize > 8) return false;

		if(tagSize > 30 - int(log2(btbSize)) || tagSize < 0) return false;

		if(fsmState > 3) return false;

		for(int i = 0 ; i < 6 ; i++)
		{
			if (btbSize == 2^i) return true;
		}

		return false;
	}

bp::bp(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned FSMState, bool isGlobalHist, bool isGlobalTable, int shared):
		btbSize(btbSize), historySize(historySize), tagSize(tagSize), fsmState(FSMState),
		isGlobalHist(isGlobalHist), isGlobalTable(isGlobalTable), Shared(shared){}

int getTagBits(unsigned btbSize, uint32_t pc){
	uint32_t tag = pc >> int(2 + log2(btbSize));
	return tag;
}

void bp::addNewBranch(uint32_t pc, uint32_t targetPc, bool taken)
{
	int bhr_index = hash_func(btbSize, pc);
	int fsm_index = calculateFsmPtr(pc);
	int pc_tag = getTagBits(btbSize, pc);
	uint32_t pc_in_list =  btb_list[bhr_index].getBranchPC();
	if(pc_in_list == pc_tag){
		btb_list[bhr_index].updateTargetPC(targetPc); //****************not sure!!!! update the old targetPc to the new one? 
		if(isGlobalTable) main_bp.global_fsm.updateFSM(fsm_index, taken);
		else btb_list[bhr_index].updateFSM(fsm_index, taken); //* update the state machine of the existing pc branch
		if(isGlobalHist) main_bp.global_history.updateHistory(taken);
		else btb_list[bhr_index].updateHistory(taken);
		return;
	}
	else{
		btb_list[bhr_index] = branch(pc_tag, targetPc, isGlobalHist, isGlobalTable, history_size, fsm_size, fsmState);
		if(isGlobalTable) main_bp.global_fsm.updateFSM(fsm_index, taken);
		else btb_list[bhr_index].updateFSM(fsm_index, taken); 
		if(isGlobalHist) main_bp.global_history.updateHistory(taken);
		else btb_list[bhr_index].updateHistory(taken);
	}
}

bool bp::nextPred(uint32_t pc)
{
	int pc_index = hash_func(btbSize, pc);
	if(btb_list[pc_index].getBranchPC() != 0){
		states* Fsm;
		if(isGlobalTable) Fsm = main_bp.global_fsm.getCurrentState();
		else Fsm = btb_list[pc_index].local_fsm.getCurrentState();
		int index = calculateFsmPtr(pc);
		if(Fsm[index]  <=  WNT) return false;
		else return true;
	}
	return false;
}

int bp::calculateMemorySize()
{
	if(isGlobalHist && isGlobalTable) return history_size + fsm_size * 2 + (VALID_BIT + tagSize + SIZE_OF_TARGET_PC) * btb_list.size();
	else if(isGlobalTable && !isGlobalHist) return fsm_size * 2 + (history_size + VALID_BIT + tagSize + SIZE_OF_TARGET_PC) * btb_list.size();
	else if(isGlobalHist && !isGlobalTable) return history_size + (fsm_size * 2 + tagSize + VALID_BIT + SIZE_OF_TARGET_PC) * btb_list.size();
	else return (history_size + fsm_size * 2 + tagSize + VALID_BIT + SIZE_OF_TARGET_PC) * btb_list.size();
}

void bp::statsUpdate(bool taken)
{
	if(!taken){
		bp_stats.flush_num++; //* update flush number only if necessary *//
	}
	bp_stats.br_num++;
}


void bp::BP_init_update(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned FSMState, bool isGlobalHist, bool isGlobalTable, int shared)
{
	history_size = historySize;
	fsm_size = calculateFsmSize(historySize);
	this->btbSize = btbSize;
	this->historySize = historySize;
	this->tagSize = tagSize;
	this->fsmState = FSMState;
	this->isGlobalHist = isGlobalHist;
	this->isGlobalTable = isGlobalTable;
	this->Shared = shared;
	this->btb_list.resize(btbSize);
	global_history = history();
	global_fsm = fsm(calculateFsmSize(historySize),FSMState);
}

/**
 * hw functions
 * 
 **/


int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState, bool isGlobalHist, bool isGlobalTable, int Shared)
{
	if(!IsdataValid(btbSize, historySize, tagSize, fsmState)) return -1;
	main_bp.BP_init_update(btbSize, historySize, tagSize, fsmState, isGlobalTable, isGlobalHist, Shared);
	return 0;
}

bool BP_predict(uint32_t pc, uint32_t *dst)
{
	int pc_index = hash_func(main_bp.btbSize, pc);
	int pc_tag = getTagBits(main_bp.btbSize, pc);
	uint32_t pc_in_list =  main_bp.btb_list[pc_index].getBranchPC();
	if(pc_in_list != pc_tag){
		*dst = pc + 4;
		return false;
	}

	if (pc_index >= main_bp.btb_list.size()){
		*dst = pc + 4;
		return false;
	}
	bool next_pred = main_bp.nextPred(pc);
	if(next_pred == false){
		*dst = pc + 4;
		return false;
	}
	*dst = main_bp.btb_list[pc_index].getTargetPC();
	return true;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst)
{
	bool next_pred;
	int pc_index = hash_func(main_bp.btbSize, pc);
	int pc_tag = getTagBits(main_bp.btbSize, pc);
	uint32_t pc_in_list =  main_bp.btb_list[pc_index].getBranchPC();
	if(pc_in_list != pc_tag){
		next_pred = false;
		main_bp.statsUpdate(taken == next_pred);
		main_bp.addNewBranch(pc, targetPc, taken);
		return;
	}
	else next_pred = main_bp.nextPred(pc);
	if(next_pred){  // check if the pc state machine shows to take the prediction or not
		main_bp.statsUpdate(taken == main_bp.nextPred(pc)); // change the SIM_stats saved in the predictor - add 1 to br_num, "false" = don't add any flush_num
		main_bp.addNewBranch(pc, targetPc, taken); // add new noot to the btb - add the pred_dst because the prediction is taken
	}
	else{
		main_bp.statsUpdate(taken == next_pred); // change the SIM_stats saved in the predictor - add 1 to br_num, "true" = add 3 to flush_num
		main_bp.addNewBranch(pc, pred_dst, taken); // add new note to the BTB - add the targetPC because the prediction is not taken
		return;
	}
	return;
}

void BP_GetStats(SIM_stats *curStats){
	curStats->br_num = main_bp.bp_stats.br_num;
	curStats->flush_num = main_bp.bp_stats.flush_num;
	curStats->size = main_bp.calculateMemorySize();
}