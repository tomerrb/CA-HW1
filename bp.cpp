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
//FSM states
enum states {SNT = 0, WNT = 1, WT = 2, ST = 3};

//initialize generic history and fsm sizes
int history_size = 0;
int fsm_size = 0;

int calculateFsmSize(unsigned historySize){
	return int(pow(2,historySize));
}

/**
 * history class
 * @arg array- an array of booleans representing wether previous branch command was taken or not
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
	void resetHistory(bool taken);
	const int historyArrToNum();
	friend class bp;
};

/**
 * history(): a history class default c'tor
 * */
history::history()
{
	array = new bool[history_size]();
	for (int i = 0 ; i < history_size ; i++){
		array[i] = 0;
	}
}

/**
 * ~history(): a history class default d'tor
 * */
history::~history()
{
	delete[] array;
}

/**
 * getHistory(): returns a ptr to beggining of history's array
 * */
const bool* history::getHistory()
{
	return this->array;
}

/**
 * getSize(): returns history's array size
 * */
int history::getSize(){
	return history_size;
}

/**
 * history(): a history class copy c'tor
 * @param hist- history object to be copied
 * */
history::history(const history& hist){
	for(int i = 0 ; i < history_size ; i++){
		array[i] = hist.array[i];
	}
}

/**
 * operator=(): a history class assignment operator
 * @param hist- history object to be copied
 * */
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

/**
 * updateHistory(): shift left the history vector values, and add new value
 * @param taken- recent history to be added
 * */
void history::updateHistory (bool taken)  // update the array - shift left
{
	for(int i = history_size - 1 ; i > 0 ; i--){
		array[i] = array[i-1];
	}
	if(taken) array[0] = 1;
	else array[0] = 0;
}
/**
 * resetHistory(): resets history's vector to 0 and add recent history
 * @param taken- recent history to be added
 * */
void history::resetHistory(bool taken){
	for(int i = 0 ; i < history_size ; i++){
		array[i] = 0;
	}
	if(taken) array[0] = 1;
}

/**
 * historyArrToNum(): convert history's vector to an integer number
 * 
 * */
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
 * @arg FSMs- an array of states used to predict branches taken or not
 * @arg fsm_num- array size
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

/**
 * fsm(): a fsm class c'tor
 * @param num_of_fsm- array size
 * @param initial_state- generic state for all fsms
 * */
fsm::fsm(int num_of_fsm = fsm_size, unsigned initial_state = WNT): fsm_num(num_of_fsm)
{
	FSMs = new states[fsm_num]();
	for (int i = 0 ; i < fsm_num ; i++){
		FSMs[i] = states(initial_state);
	}
}

/**
 * ~fsm(): a default fsm class d'tor
 *
 * */
fsm::~fsm()
{
	delete[] FSMs;
}

/**
 * operator=(): a fsm class assignment operator
 * @param Fsm- fsm object to be copied
 * */
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

/**
 * getCurrentState(): returns a ptr to beggining of fsm's array
 * */
states* fsm::getCurrentState()
{
	return this->FSMs;
}

/**
 * updateFSM(): update specified fsm according branch resolution
 * @param fsm_num- number of fsm to update
 * @param taken- branch resolution
 * */
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
 * branch class- representing a line in BTB
 * @arg branchPC- branch tag bits
 * @arg targetPC- branch target pc to jump to
 * @arg local_hist- local history. NULL if using global history
 * @arg local_fsm- local fsm. NULL if using global fsm table
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
	void resetHistory(bool taken){local_hist.resetHistory(taken);}
	branch& operator=(const branch& to_copy);
	friend class bp;
};

/**
 * branch(): a branch class c'tor
 * @param branchPC- branch tag bits
 * @param targetPC- branch target pc to jump to
 * @param isGlobalHist- true for global, false for local
 * @param isGlobalFSM- true for global, false for local
 * @param numOfHistBits- history size
 * @param numOfFSMBits- fsm size
 * @param initial_state- generic state for initializing fsm
 * */
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

/**
 * branch(): a branch class copy c'tor
 * @param to_copy- branch object to be copied
 * */
branch::branch(const branch& to_copy): branchPC(to_copy.branchPC), targetPC(to_copy.targetPC), 
									   local_hist(to_copy.local_hist), local_fsm(to_copy.local_fsm){}

/**
 * getBranchPC(): returns current branch's tag
 * */
const uint32_t branch::getBranchPC()
{
	return this->branchPC;
}

/**
 * getTargetPC(): returns current branch's target
 * */
const uint32_t branch::getTargetPC()
{
	return this->targetPC;
}

/**
 * updateTargetPC(): update current branch's target
 * @param new_PC- the updated target
 * */
void branch::updateTargetPC(const uint32_t new_PC)
{
	this->targetPC = new_PC;
}

/**
 * getTargetPC(): returns reference to current branch's history
 * */
const history& branch::getHistory()
{
	return local_hist;
}

/**
 * updateHistory(): insert new history to current branch
 * @param taken- recent history to be added
 * */
void branch::updateHistory(bool taken)
{
	local_hist.updateHistory(taken);
}

/**
 * getTargetPC(): returns reference to current branch's fsm
 * */
const fsm& branch::getFSM()
{
	return local_fsm;
}

/**
 * updateFSM(): update current branch's specified fsm according branch resolution
 * @param fsm_num- number of fsm to update
 * @param taken- branch resolution
 * */
void branch::updateFSM(int fsm_num, bool taken)
{
	local_fsm.updateFSM(fsm_num, taken);
}

/**
 * operator=(): a branch class assignment operator
 * @param to_copy- branch object to be copied
 * */
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

/**
 * Branch predictor class
 * @arg btb_list- vector of branches
 * @arg btbSize- number of rows in btb
 * @arg historySize
 * @arg tagSize
 * @arg fsmState- initial state
 * @arg isGlobalHist- true for global, false for local
 * @arg isGlobalTable- true for global, false for local
 * @arg shared- shared state- lsb/mid/not shared
 * @arg bp_stats- stats about current program- size, number of branches and number of flushes
 * @arg global_history- NULL if using local
 * @arg global_fsm- NULL if using local
 **/
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
	void statsUpdate(bool taken, bool same_address);
	int calculateFsmPtr(uint32_t pc);
	const branch& createNewBranch(unsigned branchPC, unsigned targetPC, bool taken);
	void BP_init_update(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned FSMState, bool isGlobalHist, bool isGlobalTable, int shared);
	friend bool BP_predict(uint32_t pc, uint32_t *dst);
	friend void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst);
	friend void BP_GetStats(SIM_stats *curStats);
	
};

//global branch predictor to be used during the program running
bp main_bp = bp(0,0,0,0,false,false,0);

/**
 * hash_func(): calculate given pc btb table's row
 * @param btbSize
 * @param pc
 * 
 * @return number of row suitible for pc
 * */
int hash_func(unsigned btbSize, uint32_t pc){
	uint32_t temp = pc>>2;
	int index = int(pow(2, log2(btbSize)) - 1) & temp;
	return index;
}

/**
 * calculateFsmPtr(): calculate given branch pc fsm's row
 * @param pc
 * 
 * @return number of row suitible for the branch in fsm table
 * */
int bp::calculateFsmPtr(uint32_t pc)
{
	int FsmRow = 0;
	int bhr_index = hash_func(this->btbSize, pc);
	uint32_t pc_in_list = this->btb_list[bhr_index].getBranchPC();
	int hist;
	//transform history array to number
	if(isGlobalHist){
		hist = main_bp.global_history.historyArrToNum();
	}
	else hist = this->btb_list[bhr_index].local_hist.historyArrToNum();
	// handeling different cases according to shared status
	if (this->Shared == 1) //LSB
	{
		int pcLSB = pc >> 2;
		pcLSB = pcLSB & int(pow(2, history_size) - 1);
		FsmRow = hist ^ pcLSB;
	}
	if (this->Shared == 2) //mid
	{
		int pcMB = pc >> 16;
		pcMB = pcMB & int(pow(2, history_size) - 1);
		FsmRow = hist ^ pcMB;
	}
	if (this->Shared == 0) // not shared
	{
		FsmRow = hist;		
	}
	return FsmRow;
}

/**
 * IsdataValid(): cheking initial arguments for BP constracting
 * @param btbSize
 * @param historySize
 * @param tagSize
 * @param fsmState- initial fsm's states

 * @return true if valid, false if not
 * */
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

/**
 * bp(): a bp class c'tor
 * @param btbSize
 * @param historySize
 * @param tagSize
 * @param FSMState- initial fsm's states
 * @param isGlobalHist- true for global, false for local
 * @param isGlobalTable- true for global, false for local
 * @param shared- shared state- lsb/mid/not shared
 * */
bp::bp(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned FSMState, bool isGlobalHist, bool isGlobalTable, int shared):
		btbSize(btbSize), historySize(historySize), tagSize(tagSize), fsmState(FSMState),
		isGlobalHist(isGlobalHist), isGlobalTable(isGlobalTable), Shared(shared){}

/**
 * getTagBits(): calculate given pc tag's
 * @param btbSize
 * @param pc
 * @param tag_size
 * 
 * @return pc's tag
 * */
int getTagBits(unsigned btbSize, uint32_t pc, int tag_size){
	uint32_t tag = pc >> int(2 + log2(btbSize));
	return tag & int(pow(2, tag_size) - 1);
}

/**
 * addNewBranch(): finds current branch's row. if already exist- updating it, if not- constract a new branch and inserting it to btb
 * @param pc- new branch pc
 * @param targetPC- new branch target
 * @param taken- new branch recent history
 * */
void bp::addNewBranch(uint32_t pc, uint32_t targetPc, bool taken)
{
	int bhr_index = hash_func(btbSize, pc);
	int fsm_index = calculateFsmPtr(pc);
	int pc_tag = getTagBits(btbSize, pc, main_bp.tagSize);
	uint32_t pc_in_list =  btb_list[bhr_index].getBranchPC();
	if(pc_in_list == pc_tag) //current branch is already in the btb, needs to be updated
	{ 
		btb_list[bhr_index].updateTargetPC(targetPc); 
		if(isGlobalTable) main_bp.global_fsm.updateFSM(fsm_index, taken);
		else btb_list[bhr_index].updateFSM(fsm_index, taken); //* update the state machine of the existing pc branch
		if(isGlobalHist) main_bp.global_history.updateHistory(taken);
		else btb_list[bhr_index].updateHistory(taken);
		return;
	}
	else   //un-familliar branch- remove old one and insert new branch
	{ 
		btb_list[bhr_index] = branch(pc_tag, targetPc, isGlobalHist, isGlobalTable, history_size, fsm_size, fsmState);
		if(isGlobalTable && isGlobalHist) main_bp.global_fsm.updateFSM(fsm_index, taken);
		else if(isGlobalTable && !isGlobalHist) main_bp.global_fsm.updateFSM(0, taken);
		else if(isGlobalHist) btb_list[bhr_index].updateFSM(fsm_index, taken); 
		else btb_list[bhr_index].updateFSM(0, taken); 

		if(isGlobalHist) main_bp.global_history.updateHistory(taken);
		else btb_list[bhr_index].resetHistory(taken);
	}
}

/**
 * nextPred(): returns given branch's prediction
 * @param pc
 * 
 * @return true if taken, false if not-taken
 * */
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

/**
 * calculateMemorySize(): calculate current program's bp memory size
 * 
 * @return memory size (bits)
 * */
int bp::calculateMemorySize()
{
	if(isGlobalHist && isGlobalTable) return history_size + fsm_size * 2 + (VALID_BIT + tagSize + SIZE_OF_TARGET_PC) * btb_list.size();
	else if(isGlobalTable && !isGlobalHist) return fsm_size * 2 + (history_size + VALID_BIT + tagSize + SIZE_OF_TARGET_PC) * btb_list.size();
	else if(isGlobalHist && !isGlobalTable) return history_size + (fsm_size * 2 + tagSize + VALID_BIT + SIZE_OF_TARGET_PC) * btb_list.size();
	else return (history_size + fsm_size * 2 + tagSize + VALID_BIT + SIZE_OF_TARGET_PC) * btb_list.size();
}

/**
 * statsUpdate(): update current program's bp stats- number of branches and flushes
 * @param taken- branch's resolution
 * @param same_address- true if target pc is pc + 4
 * 
 * */
void bp::statsUpdate(bool taken, bool same_address)
{
	if(!taken || !same_address){
		bp_stats.flush_num++; //* update flush number only if necessary *//
	}
	bp_stats.br_num++;
}

/**
 * BP_init_update(): an assistant funcion for initialization
 * @param btbSize
 * @param historySize
 * @param tagSize
 * @param FSMState- initial fsm's states
 * @param isGlobalHist- true for global, false for local
 * @param isGlobalTable- true for global, false for local
 * @param shared- shared state- lsb/mid/not shared
 * */
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

/************************************
 * 
 * HW functions- Interface functions
 * 
 ************************************/

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState, bool isGlobalHist, bool isGlobalTable, int Shared)
{
	if(!IsdataValid(btbSize, historySize, tagSize, fsmState)) return -1;
	main_bp.BP_init_update(btbSize, historySize, tagSize, fsmState, isGlobalHist, isGlobalTable, Shared);
	return 0;
}

bool BP_predict(uint32_t pc, uint32_t *dst)
{
	int pc_index = hash_func(main_bp.btbSize, pc);
	int pc_tag = getTagBits(main_bp.btbSize, pc, main_bp.tagSize);
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
	int pc_tag = getTagBits(main_bp.btbSize, pc, main_bp.tagSize);
	uint32_t pc_in_list =  main_bp.btb_list[pc_index].getBranchPC();
	if(pc_in_list != pc_tag){
		next_pred = false;
		main_bp.statsUpdate(taken == next_pred, !(taken && targetPc != pred_dst));
		main_bp.addNewBranch(pc, targetPc, taken);
		return;
	}
	else next_pred = main_bp.nextPred(pc);
	if(next_pred){  // check if the pc state machine shows to take the prediction or not
		main_bp.statsUpdate(taken == main_bp.nextPred(pc), !(taken && targetPc != pred_dst));
		main_bp.addNewBranch(pc, targetPc, taken); // add new noot to the btb - add the pred_dst because the prediction is taken
	}
	else{
		main_bp.statsUpdate(taken == next_pred, !(taken && targetPc != pred_dst));
		main_bp.addNewBranch(pc, targetPc, taken); // add new note to the BTB - add the targetPC because the prediction is not taken
		return;
	}
	return;
}

void BP_GetStats(SIM_stats *curStats){
	curStats->br_num = main_bp.bp_stats.br_num;
	curStats->flush_num = main_bp.bp_stats.flush_num;
	curStats->size = main_bp.calculateMemorySize();
}