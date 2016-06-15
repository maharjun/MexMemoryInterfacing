#include <utility>
#include "FlatVectTree.hpp"

typedef uint32_t T;
typedef std::enable_if< std::is_arithmetic<T>::value >::type B;

/////////////////////////////////////////////////
// PRIVATE HELPER FUNCTIONS  ////////////////////
/////////////////////////////////////////////////

template<typename T, class FVT_Al, class B>
uint32_t FlatVectTree<T, FVT_Al, B>::getAppendInsertDepth(uint32_t GivenInsertDepth, uint32_t AppendTreeDepth) const {
	// This function does the following:
	//
	// 1. Validates GivenInsertDepth in the context of AppendTreeDepth and CurrDepth.
	// 2. Assigns the default value to GivenInsertDepth if it is uint32_t(-1)
	//
	// It returns the (Default or previously assigned) value of GivenInsertDepth
	//
	// This function calculates default values as  according to the append scheme of
	// things.

	uint32_t CurrDepth = this->depth();

	// Initializing Default Values and validating GivenInsertDepth
	if (GivenInsertDepth == -1) {
		if (CurrDepth > 0)
			if (AppendTreeDepth <= CurrDepth)
				GivenInsertDepth = CurrDepth - AppendTreeDepth;
			else
				WriteException(
					FV_ExCodes::FV_INVALID_APPEND,
					"The Depth of array to be appended (%d) exceeds the current depth (%d) of the Non-Empty flat cell array",
					AppendTreeDepth, CurrDepth
				);
		else
			GivenInsertDepth = 0;
	}
	else {
		if (GivenInsertDepth < 0)
			WriteException(
				FV_ExCodes::FV_INVALID_APPEND,
				"GivenInsertDepth (%d) must be positive",
				GivenInsertDepth
			);
		else if (CurrDepth > 0 && GivenInsertDepth + AppendTreeDepth > CurrDepth)
			WriteException(
				FV_ExCodes::FV_INVALID_APPEND,
				"GivenInsertDepth (%d) must not exceed this->depth()-depth of given array (%d)",
				GivenInsertDepth,
				CurrDepth - AppendTreeDepth
			);
	}
	return GivenInsertDepth;
};

template<typename T, class FVT_Al, class B>
uint32_t FlatVectTree<T, FVT_Al, B>::getActualInsertDepth(uint32_t InsertDepth, uint32_t GivenDepth) const {

	uint32_t CurrDepth = this->depth();

	uint32_t ActualInsertDepth = 0;
	if (CurrDepth == 0) {
		// if FlatVectTree is empty.
		ActualInsertDepth = 0;
	}
	else if (InsertDepth > 0) {
		// find actual insert depth in case the given level doesn't exist
		if (this->PartitionIndex[0].size() == 1) {
			ActualInsertDepth = 0;
		}
		else {
			for (ActualInsertDepth = 0; ActualInsertDepth < InsertDepth; ++ActualInsertDepth) {
				size_t tempSize = PartitionIndex[ActualInsertDepth].size();
				if (this->PartitionIndex[ActualInsertDepth][tempSize - 1] == this->PartitionIndex[ActualInsertDepth][tempSize - 2]) {
					ActualInsertDepth++;
					break;
				}
			}
		}
	}
	return ActualInsertDepth;
}

/////////////////////////////////////////////////
// ASSIGNMENT FUNCTIONS      ////////////////////
/////////////////////////////////////////////////
template<typename T, class FVT_Al, class B>
template<class AlSub, class Al, class AlData>
inline void FlatVectTree<T, FVT_Al, B>::assign(const MexVector<MexVector<uint32_t, AlSub>, Al> &PartitionIndexIn, const MexVector<T, AlData> & DataIn, bool ActualCopy)
{
	if (FlatVectTree<T>::isValidFVT(PartitionIndexIn, DataIn)) {
		if (ActualCopy) {
			PartitionIndex = PartitionIndexIn;
			Data = DataIn;
		}
		else {
			uint32_t TreeDepth = PartitionIndexIn.size();
			PartitionIndex.resize(TreeDepth);
			for (uint32_t i = 0; i < TreeDepth; ++i) {
				PartitionIndex[i].assign(PartitionIndexIn[i].size(), PartitionIndexIn[i].begin(), false);
			}
			Data.assign(DataIn.size(), DataIn.begin(), false);
		}
	}
	else {
		WriteException(
			FV_ExCodes::FV_INVALID_APPEND,
			"The Given PartialIndexIn, DataIn do not represent a valid FlatVectTree"
			);
	}

}

template<typename T, class FVT_Al, class B>
template<class FVT_Al2>
inline void FlatVectTree<T, FVT_Al, B>::assign(const FlatVectTree<T, FVT_Al2> &FlatVectTreeIn, bool ActualCopy)
{
	if (ActualCopy) {
		PartitionIndex = FlatVectTreeIn.PartitionIndex;
		Data = FlatVectTreeIn.Data;
	}
	else {
		uint32_t TreeDepth = FlatVectTreeIn.PartitionIndex.size();
		PartitionIndex.resize(TreeDepth);
		for (uint32_t i = 0; i < TreeDepth; ++i) {
			PartitionIndex[i].assign(FlatVectTreeIn.PartitionIndex[i].size(), FlatVectTreeIn.PartitionIndex[i].begin(), false);
		}
		Data.assign(FlatVectTreeIn.Data.size(), FlatVectTreeIn.Data.begin(), false);
	}
}

/////////////////////////////////////////////////
// APPEND FUNCTIONS          ////////////////////
/////////////////////////////////////////////////

// Fast Append Functions
// =====================

// ## Copy Versions ##
template<typename T, class FVT_Al, class B>
template<class Al>
inline void FlatVectTree<T, FVT_Al, B>::appendFast(const MexVector<T, Al> &VectIn) {

	/*
	    Template specialization (recursion termination step) for appendFast
	*/

	uint32_t OldSize = this->Data.size();
	uint32_t NElems = VectIn.size();
	this->Data.push_size(NElems);
	this->Data.copyArray(OldSize, VectIn.begin(), NElems);

}

template<typename T, class FVT_Al, class B>
template<typename SubElemT, class AlSub, class Al>
inline void FlatVectTree<T, FVT_Al, B>::appendFast(const MexVector<MexVector<SubElemT, AlSub>, Al > &VectTreeIn) {
	/*
	   AppendFast simply appends the given Tree to its required height without
	   doing any for of validations, default calculations. it also does not edit 
	   in any way the levels higher than this->depth() - (depth of VectTreeIn).
	   The above edit is expected to be done in any function which calls apendFast.
	   Know that if a valid such edit is not done, the appendFast makes the 
	   FlatCell invalid.
	*/

	uint32_t InsertDepth = this->depth() - getTreeInfo<decltype(VectTreeIn)>::depth;
	uint32_t NElems      = VectTreeIn.size();
	
	for (uint32_t i = 0; i < NElems; ++i) {
		uint32_t NSubElems = VectTreeIn[i].size();
		appendFast(VectTreeIn[i]);
		PartitionIndex[InsertDepth].push_back(PartitionIndex[InsertDepth].last() + NSubElems);
	}
}

// ## Move Versions ##
template<typename T, class FVT_Al, class B>
template<class Al>
inline void FlatVectTree<T, FVT_Al, B>::appendFast(MexVector<T, Al> &&VectIn) {
	/*
	   Template specialization (recursion termination step) for appendFast move version
	*/

	uint32_t OldSize = this->Data.size();
	uint32_t NElems = VectIn.size();
	this->Data.push_size(NElems);
	this->Data.copyArray(OldSize, VectIn.begin(), NElems);
	VectIn.clear();
	VectIn.trim();
}

template<typename T, class FVT_Al, class B>
template<typename SubElemT, class AlSub, class Al>
inline void FlatVectTree<T, FVT_Al, B>::appendFast(MexVector<MexVector<SubElemT, AlSub>, Al > &&VectTreeIn) {
	/*
	AppendFast simply appends the given Tree to its required height without
	doing any for of validations, default calculations. it also does not edit
	in any way the levels higher than this->depth() - (depth of VectTreeIn).
	The above edit is expected to be done in any function which calls apendFast.
	Know that if a valid such edit is not done, the appendFast makes the
	FlatCell invalid.
	*/

	uint32_t InsertDepth = this->depth() - getTreeInfo<decltype(VectTreeIn)>::depth;
	uint32_t NElems = VectTreeIn.size();

	for (uint32_t i = 0; i < NElems; ++i) {
		uint32_t NSubElems = VectTreeIn[i].size();
		appendFast(VectTreeIn[i]);
		PartitionIndex[InsertDepth].push_back(PartitionIndex[InsertDepth].last() + NSubElems);
	}

	// Deallocating VectTreeIn
	VectTreeIn.clear();
	VectTreeIn.trim();
}

// Actual Append Functions
// =======================
template<typename T, class FVT_Al, class B>
template<typename SubElemT, class Al>
inline void FlatVectTree<T, FVT_Al, B>::append(const MexVector<SubElemT, Al> &VectTreeIn, uint32_t InsertDepth) {
	/* 
	   This function appends the given MexVectIn at the specified InsertDepth
	   
	   DEFAULT VALUES & CONSTRAINTS:
	   
	   1. The the case that the current FlatTreeVect is non-empty (i.e. PartitionIndex.size() > 0)
	      The following constraint is required to be followed else an exception is thrown.
	   
	      InsertDepth + getTreeInfo<MexVector<MexVector<T> > >::value < this->depth();
	   
	   2. If InsertDepth is Not Specified, and FlatTreeVect is Non-empty, it is calculated according
	      to the constraint
	   
	      InsertDepth + getTreeInfo<MexVector<MexVector<T> > >::value  = this->depth();
	   
	      If getTreeInfo<MexVector<MexVector<T> > >::value > this->depth(), then an exception is 
	      thrown.
	   
	   3. If InsertDepth is not specified, and FlatTreeVect is Empty, then InsertDepth defaults to 0

	   4. If FlatTreeVect is Empty, its new depth is calculated to be 
	   
	      InsertDepth + getTreeInfo<MexVector<MexVector<T> > >::value >
	   
	   EXAMPLES:
	   
	   Using MATLAB notation to denote a vector of vectors as a cell array, we have the following:
	   
	   1. FlatCellArray = {
	                         {
	                            [1 2 3],
	                            [2 3],
	                            [4 5 6]
	                         }
	                         {
	                            [1 2 4],
	                            [5 6],
	                            [7]
	                         }
	                      }
	      We see that FlatCellArray.depth() = 2
	   
	      Consider the following cases
	      ===================================================|======================================
	                                                         |
	      FLatCellArray.append(                              |     FLatCellArray.append(
	                           {                             |                          {
	                              [1 9 8]                    |                             [1 9 8]
	                           }                             |                          }
	                          );                             |                          , 0
	      equiv to FlatCellArray.append(..., 2-1);           |                         );
	                                                         |
	      FlatCellArray = {                                  |     FlatCellArray = {
	                         {                               |                        {
	                            [1 2 3],                     |                           [1 2 3],
	                            [2 3],                       |                           [2 3],
	                            [4 5 6]                      |                           [4 5 6]
	                         }                               |                        }
	                         {                               |                        {
	                            [1 2 4],                     |                           [1 2 4],
	                            [5 6],                       |                           [5 6],
	                            [7],                         |                           [7],
	                            [1 9 8]   // NEW ADDITION    |                        }
	                         }                               |                        {
	                      }                                  |                           [1 9 8]   // NEW ADDITION
	                                                         |                        }
	                                                         |                     }
          ===================================================|======================================
	      
	*/

	// Intializing Depth Variables
	uint32_t CurrDepth = this->depth();
	uint32_t GivenDepth = getTreeInfo<decltype(VectTreeIn)>::depth;

	InsertDepth = getAppendInsertDepth(InsertDepth, GivenDepth);
	uint32_t ActualInsertDepth = getActualInsertDepth(InsertDepth, GivenDepth);

	// Initialize FVT if empty
	if (this->istrulyempty()) {
		this->setDepth(InsertDepth + GivenDepth);
	}

	// Perform Fast Append
	appendFast(VectTreeIn);

	// If The vector is inserted on a Level higher than CurrDepth - GivenDepth
	// then an additional entry needs to be made for all the levels from the 
	// CurrDepth - GivenDepth - 1 to the ActualInsertDepth.
	if (ActualInsertDepth < CurrDepth - GivenDepth) {
		if (GivenDepth == 0)
			PartitionIndex[CurrDepth - 1].push_back(Data.size());
		else
			PartitionIndex[CurrDepth - GivenDepth - 1].push_back(PartitionIndex[CurrDepth - GivenDepth].size() - 1);
		
		for (uint32_t i = CurrDepth - GivenDepth - 1; i --> ActualInsertDepth ;) {
			PartitionIndex[i].push_back(PartitionIndex[i + 1].size() - 1); // Discounting BTE Element
		}
	}
	
	// The Beyond-The_End element for the level above the insert 
	// level needs to be updated (assuming Such a level exists)
	if (ActualInsertDepth > 0 && ActualInsertDepth != CurrDepth)
		PartitionIndex[ActualInsertDepth - 1].last() = PartitionIndex[ActualInsertDepth].size() - 1;
	else if (ActualInsertDepth > 0 && ActualInsertDepth == CurrDepth) {
		PartitionIndex[ActualInsertDepth - 1].last() = Data.size();
	}
}

template<typename T, class FVT_Al, class B>
template<typename SubElemT, class Al>
inline void FlatVectTree<T, FVT_Al, B>::append(MexVector<SubElemT, Al> &&VectTreeIn, uint32_t InsertDepth) {
	/* 
	   This function appends the given MexVectIn at the specified InsertDepth
	   
	   DEFAULT VALUES & CONSTRAINTS:
	   
	   1. The the case that the current FlatTreeVect is non-empty (i.e. PartitionIndex.size() > 0)
	      The following constraint is required to be followed else an exception is thrown.
	   
	      InsertDepth + getTreeInfo<MexVector<MexVector<T> > >::value < this->depth();
	   
	   2. If InsertDepth is Not Specified, and FlatTreeVect is Non-empty, it is calculated according
	      to the constraint
	   
	      InsertDepth + getTreeInfo<MexVector<MexVector<T> > >::value  = this->depth();
	   
	      If getTreeInfo<MexVector<MexVector<T> > >::value > this->depth(), then an exception is 
	      thrown.
	   
	   3. If InsertDepth is not specified, and FlatTreeVect is Empty, then InsertDepth defaults to 0

	   4. If FlatTreeVect is Empty, its new depth is calculated to be 
	   
	      InsertDepth + getTreeInfo<MexVector<MexVector<T> > >::value >
	   
	   EXAMPLES:
	   
	   Using MATLAB notation to denote a vector of vectors as a cell array, we have the following:
	   
	   1. FlatCellArray = {
	                         {
	                            [1 2 3],
	                            [2 3],
	                            [4 5 6]
	                         }
	                         {
	                            [1 2 4],
	                            [5 6],
	                            [7]
	                         }
	                      }
	      We see that FlatCellArray.depth() = 2
	   
	      Consider the following cases
	      ===================================================|======================================
	                                                         |
	      FLatCellArray.append(                              |     FLatCellArray.append(
	                           {                             |                          {
	                              [1 9 8]                    |                             [1 9 8]
	                           }                             |                          }
	                          );                             |                          , 0
	      equiv to FlatCellArray.append(..., 2-1);           |                         );
	                                                         |
	      FlatCellArray = {                                  |     FlatCellArray = {
	                         {                               |                        {
	                            [1 2 3],                     |                           [1 2 3],
	                            [2 3],                       |                           [2 3],
	                            [4 5 6]                      |                           [4 5 6]
	                         }                               |                        }
	                         {                               |                        {
	                            [1 2 4],                     |                           [1 2 4],
	                            [5 6],                       |                           [5 6],
	                            [7],                         |                           [7],
	                            [1 9 8]   // NEW ADDITION    |                        }
	                         }                               |                        {
	                      }                                  |                           [1 9 8]   // NEW ADDITION
	                                                         |                        }
	                                                         |                     }
          ===================================================|======================================
	      
	*/

	// Intializing Depth Variables
	uint32_t CurrDepth = this->depth();
	uint32_t GivenDepth = getTreeInfo<decltype(VectTreeIn)>::depth;

	InsertDepth = getAppendInsertDepth(InsertDepth, GivenDepth);
	uint32_t ActualInsertDepth = getActualInsertDepth(InsertDepth, GivenDepth);

	// Initialize FVT if empty
	if (this->istrulyempty()) {
		this->setDepth(InsertDepth + GivenDepth);
	}

	// Perform Fast Append
	appendFast(std::move(VectTreeIn));

	// If The vector is inserted on a Level higher than CurrDepth - GivenDepth
	// then an additional entry needs to be made for all the levels from the
	// CurrDepth - GivenDepth - 1 to the ActualInsertDepth.
	if (ActualInsertDepth < CurrDepth - GivenDepth) {
		if (GivenDepth == 0)
			PartitionIndex[CurrDepth - 1].push_back(Data.size());
		else
			PartitionIndex[CurrDepth - GivenDepth - 1].push_back(PartitionIndex[CurrDepth - GivenDepth].size() - 1);

		for (uint32_t i = CurrDepth - GivenDepth - 1; i --> ActualInsertDepth ;) {
			PartitionIndex[i].push_back(PartitionIndex[i + 1].size() - 1); // Discounting BTE Element
		}
	}

	// The Beyond-The_End element for the level above the insert 
	// level needs to be updated (assuming Such a level exists)
	if (ActualInsertDepth > 0 && ActualInsertDepth != CurrDepth)
		PartitionIndex[ActualInsertDepth - 1].last() = PartitionIndex[ActualInsertDepth].size() - 1;
	else if (ActualInsertDepth > 0 && ActualInsertDepth == CurrDepth) {
		PartitionIndex[ActualInsertDepth - 1].last() = Data.size();
	}
}

template<typename T, class FVT_Al, class B>
template<class Al>
inline void FlatVectTree<T, FVT_Al, B>::append(const FlatVectTree<T, Al> &VectTreeIn, uint32_t InsertDepth) {
	/*
	 * This function is used to append a FlatVectTree instead of a VectVect.
	 * The semantics of this operation are identical to the other append functions
	 */

	// Initializing Depth Variables
	uint32_t CurrDepth = this->depth();
	uint32_t GivenDepth = VectTreeIn.depth();

	InsertDepth = getAppendInsertDepth(InsertDepth, GivenDepth);
	uint32_t ActualInsertDepth = getActualInsertDepth(InsertDepth, GivenDepth);

	// Initialize FVT if empty
	if (this->istrulyempty()) {
		this->setDepth(InsertDepth + GivenDepth);
		CurrDepth = InsertDepth + GivenDepth;
	}

	// Perform Fast Append
	for(uint32_t i=0; i<GivenDepth; ++i) {
		auto &CurrentFVTPartition = PartitionIndex[CurrDepth - GivenDepth + i];
		auto &AppendFVTPartition = VectTreeIn.PartitionIndex[i];
		auto CurrPartitionLastIndex = PartitionIndex[CurrDepth - GivenDepth + i].last();
		// Ignore the first element of AppendFVTPartition as hat corresponds to
		// the BLE of CurrentFVTPartition
		for(uint32_t j=1; j < AppendFVTPartition.size(); ++j) {
			CurrentFVTPartition.push_back(CurrPartitionLastIndex + AppendFVTPartition[j]);
		}
	}
	Data.insert(Data.size(), VectTreeIn.Data);

	// If The vector is inserted on a Level higher than CurrDepth - GivenDepth
	// then an additional entry needs to be made for all the levels from the
	// CurrDepth - GivenDepth - 1 to the ActualInsertDepth.
	if (ActualInsertDepth < CurrDepth - GivenDepth) {
		if (GivenDepth == 0)
			PartitionIndex[CurrDepth - 1].push_back(Data.size());
		else
			PartitionIndex[CurrDepth - GivenDepth - 1].push_back(PartitionIndex[CurrDepth - GivenDepth].size() - 1);

		for (uint32_t i = CurrDepth - GivenDepth - 1; i --> ActualInsertDepth ;) {
			PartitionIndex[i].push_back(PartitionIndex[i + 1].size() - 1); // Discounting BTE Element
		}
	}

	// The Beyond-The_End element for the level above the insert
	// level needs to be updated (assuming Such a level exists)
	if (ActualInsertDepth > 0 && ActualInsertDepth != CurrDepth)
		PartitionIndex[ActualInsertDepth - 1].last() = PartitionIndex[ActualInsertDepth].size() - 1;
	else if (ActualInsertDepth > 0 && ActualInsertDepth == CurrDepth) {
		PartitionIndex[ActualInsertDepth - 1].last() = Data.size();
	}
}

/////////////////////////////////////////////////
// PUSH_BACK FUNCTIONS       ////////////////////
/////////////////////////////////////////////////

template<typename T, class FVT_Al, class B>
template<typename SubElemT, class Al>
inline void FlatVectTree<T, FVT_Al, B>::push_back(const MexVector<SubElemT, Al> &VectTreeIn) {
	/*
	   push_back is like append except that the default calculated InsertDepth is one higher
	   than append. The InsertDepth is not taken as argument as it is expected that the user
	   use append in such case.

	   2. If FlatTreeVect is Non-empty, it is calculated according to the constraint
	   
	      InsertDepth + 1 + depth_of_VectTreeIn = this->depth();
	   
	      If depth_of_VectTreeIn > this->depth() - 1, then an exception is thrown.
	   
	   3. FlatTreeVect is Empty, then InsertDepth defaults to 1
	   
	   4. If FlatTreeVect is Empty, its new depth is calculated to be 
	   
	      1 + depth_of_VectTreeIn

	   Intuitively, push_back packages the given MexVector<MexVector<T> > into an element of a higher
	   depth Vector Tree and then pushes that element into the appropriate level.
	*/

	// Intializing Depth Variables
	uint32_t CurrDepth = this->depth();
	uint32_t GivenDepth = getTreeInfo<decltype(VectTreeIn)>::depth;
	uint32_t InsertDepth = 0;

	// Initializing Default Values and validating InsertDepth
	if (CurrDepth > 0)
		if (GivenDepth <= CurrDepth - 1)
			InsertDepth = CurrDepth - GivenDepth - 1;
		else
			WriteException(
				FV_ExCodes::FV_INVALID_APPEND,
				"The Depth of array to be pushed back (%d) exceeds the (1 + current depth) (%d) of the Non-Empty flat cell array",
				GivenDepth, 1 + CurrDepth
				);
	else
		InsertDepth = 1;

	append(VectTreeIn, InsertDepth);
}

template<typename T, class FVT_Al, class B>
template<typename SubElemT, class Al>
inline void FlatVectTree<T, FVT_Al, B>::push_back(MexVector<SubElemT, Al> &&VectTreeIn) {
	/* 
	   This performs push_back and deallocates memory in VectTreeIn
	*/

	// Initializing Depth Variables
	uint32_t CurrDepth = this->depth();
	uint32_t GivenDepth = getTreeInfo<decltype(VectTreeIn)>::depth;
	uint32_t InsertDepth = 0;

	// Initializing Default Values and validating InsertDepth
	if (CurrDepth > 0)
	if (GivenDepth <= CurrDepth - 1)
		InsertDepth = CurrDepth - GivenDepth - 1;
	else
		WriteException(
			FV_ExCodes::FV_INVALID_APPEND,
			"The Depth of array to be pushed back (%d) exceeds the (1 + current depth) (%d) of the Non-Empty flat cell array",
			GivenDepth, 1 + CurrDepth
		);
	else
		InsertDepth = 1;

	append(std::move(VectTreeIn), InsertDepth);
}

template<typename T, class FVT_Al, class B>
template<class Al>
inline void FlatVectTree<T, FVT_Al, B>::push_back(FlatVectTree<T, Al> &VectTreeIn) {
	/*
	   This performs push_back and deallocates memory in VectTreeIn
	*/

	// Initializing Depth Variables
	uint32_t CurrDepth = this->depth();
	uint32_t GivenDepth = VectTreeIn.depth();
	uint32_t InsertDepth = 0;

	// Initializing Default Values and validating InsertDepth
	if (CurrDepth > 0)
	if (GivenDepth <= CurrDepth - 1)
		InsertDepth = CurrDepth - GivenDepth - 1;
	else
		WriteException(
			FV_ExCodes::FV_INVALID_APPEND,
			"The Depth of array to be pushed back (%d) exceeds the (1 + current depth) (%d) of the Non-Empty flat cell array",
			GivenDepth, 1 + CurrDepth
		);
	else
		InsertDepth = 1;

	append(VectTreeIn, InsertDepth);
}

/////////////////////////////////////////////////
// GET VECTOR TREE FUNCTIONS ////////////////////
/////////////////////////////////////////////////

template<typename T, class FVT_Al, class B>
template<class Al>
inline void FlatVectTree<T, FVT_Al, B>::getVectTreeFromInds(MexVector<T, Al> &VectTreeOut, uint32_t Level, uint32_t LevelIndex) {

	// Validate compatibility of depth
	uint32_t OutDepth = 0;
	uint32_t InDepth = this->depth() - Level - 1; // Since we are taking an element of Level

	if (InDepth != OutDepth)
		WriteException(
			FV_ExCodes::FV_INVALID_FETCH,
			"The depth of the cell array requested (%d) doesnt match the depth of the MexVector Tree given (%d)",
			OutDepth, InDepth
		);
	
	// Validate LevelIndex
	if (LevelIndex >= PartitionIndex[Level].size() - 1) {
		WriteException(
			FV_ExCodes::FV_INVALID_FETCH,
			"The index of cell array requested (%d) exceeds the (size of the Level  - 1 = %d)",
			LevelIndex, PartitionIndex[Level].size() - 2
			);
	}

	// Returning Cell Array
	uint32_t VectSize = PartitionIndex[Level][LevelIndex + 1] - PartitionIndex[Level][LevelIndex];
	VectTreeOut.resize(VectSize);
	VectTreeOut.copyArray(0, Data.begin() + PartitionIndex[Level][LevelIndex], VectSize);
}

template<typename T, class FVT_Al, class B>
template<typename SubElemT, class AlSub, class Al>
inline void FlatVectTree<T, FVT_Al, B>::getVectTreeFromInds(MexVector<MexVector<SubElemT, AlSub>, Al> &VectTreeOut, uint32_t Level, uint32_t LevelIndex) {
	
	// Validate compatibility of depth
	uint32_t OutDepth = getTreeInfo<decltype(VectTreeOut)>::depth;
	uint32_t InDepth = this->depth() - Level - 1; // Since we are taking an element of Level

	if (InDepth != OutDepth)
		WriteException(
			FV_ExCodes::FV_INVALID_FETCH,
			"The depth of the cell array requested (%d) doesnt match the depth of the MexVector Tree given (%d)",
			InDepth, OutDepth
			);

	// Validate LevelIndex
	if (LevelIndex >= PartitionIndex[Level].size() - 1) {
		WriteException(
			FV_ExCodes::FV_INVALID_FETCH,
			"The index of cell array requested (%d) exceeds the (size of the Level  - 1 = %d)",
			LevelIndex, PartitionIndex[Level].size() - 2
			);
	}

	uint32_t NElems = PartitionIndex[Level][LevelIndex + 1] - PartitionIndex[Level][LevelIndex];
	VectTreeOut.resize(NElems);

	for (uint32_t i = 0; i < NElems; ++i) {
		uint32_t CurrElemIndex = PartitionIndex[Level][LevelIndex] + i;
		getVectTreeFromInds(VectTreeOut[i], Level + 1, CurrElemIndex);
	}
}

// Get Vector Tree
template<typename T, class FVT_Al, class B>
template<typename SubElemT, class Al, class AlInds>
inline void FlatVectTree<T, FVT_Al, B>::getVectTree(MexVector<SubElemT, Al> &VectTreeOut, const MexVector<uint32_t, AlInds> &Indices) {

	// Validate Indices.
	if (Indices.size() > this->depth()) {
		WriteException(
			FV_ExCodes::FV_INVALID_FETCH,
			"The size of Indices (%d) must not exceed the depth of the FlatVectTree (%d)",
			Indices.size(), this->depth()
		);
	}

	// Initializing required variables
	uint32_t IndicesSize = Indices.size();
	uint32_t CurrDepth = this->depth();

	// Special operation in case Indices is empty
	// in this case, return entire cell array
	if (Indices.isempty()) {
		uint32_t VectTreeDepth = getTreeInfo<decltype(VectTreeOut)>::depth;
		if (VectTreeDepth != this->depth()) {
			WriteException(
				FV_ExCodes::FV_INVALID_FETCH,
				"The depth of the cell array requested (%d) doesnt match the depth of the MexVector Tree given (%d)",
				this->depth(), VectTreeDepth
				);
		}
		else {
			uint32_t NElems = PartitionIndex[0].size() - 1;
			VectTreeOut.resize(NElems);
			for (uint32_t i = 0; i < NElems; ++i) {
				getVectTreeFromInds(VectTreeOut[i], 0, i);
			}
		}
	}
	// If Not, then Finding LevelIndex and Level and call getVectTreeFromInds
	else {
		uint32_t Level = 0;
		uint32_t LevelIndex = 0;

		for (uint32_t i = 0; i < IndicesSize; ++i, ++Level) {
			uint32_t CurrentLevelSize = PartitionIndex[Level].size() - 1;
			if (Indices[Level] < CurrentLevelSize) {
				LevelIndex = PartitionIndex[Level][LevelIndex + Indices[Level]];
			}
			else
				WriteException(
					FV_ExCodes::FV_INVALID_FETCH,
					"At Level %d, Indices[%d] = %d exceeds the size of the level (%d)",
					Level, Level, Indices[Level], CurrentLevelSize
					);
		}
		--Level; // Level becomes one more in the last increment of for loop

		getVectTreeFromInds(VectTreeOut, Level, LevelIndex);
	}
}

template<typename T, class FVT_Al, class B>
template<typename SubElemT, class Al>
inline void FlatVectTree<T, FVT_Al, B>::getVectTree(MexVector<SubElemT, Al> &VectTreeOut, uint32_t NIndices, ...) {

	// Converting Indices into vector
	MexVector<uint32_t> Indices(NIndices);
	std::va_list Args;
	va_start(Args, NIndices);
	for (uint32_t i = 0; i < NIndices; ++i) {
		Indices[i] = va_arg(Args, uint32_t);
	}
	va_end(Args);

	getVectTree(VectTreeOut, Indices);
}

/////////////////////////////////////////////////
// PROPERTY ASSIGNMENT FUNCTIONS ////////////////
/////////////////////////////////////////////////

template<typename T, class FVT_Al, class B>
inline bool FlatVectTree<T, FVT_Al, B>::setDepth(uint32_t NewDepth)
{
	/*
	   This function sets the depth of the FlatTreeVect to NewDepth given 
	   the following conditions

	     1. The Current Depth of NewDepth is 0. - 
		    
			In this case, the array is initialized to an empty FlatTreeVect
			of depth = NewDepth i.e. PartitionIndex.size() = NewDepth and
			Each vector of PartitionIndex is a single element vector containing 
			0. Data is empty

		 2. The Current Depth is not zero but less than the New Depth -
		    
			In this case, the FlatVectTree is pushed deeper. If the FlatVectArray
			is not empty, then the higher levels will have only a single element
			which contains the original VectTree as an element. If the FVA is
			empty then the new, deeper FVA will also be empty.

		 3. The Current Depth is greater than the New Depth 
		    
			In this case, if NewDepth is 0, then the setting of depth succeeds 
			only if FlatVectArray is empty (all levels contain no elements).

			if NewDepth > 0, then we require that all levels from 0 to OldDepth 
			- NewDepth - 1 have at most one element, in which case, the elements
			of the single element of the level OldDepth - NewDepth - 1 are taken
			to be the top level elements of the new FlatVectArray.
	*/

	uint32_t OldDepth = this->depth();
	if (OldDepth == 0) {
		PartitionIndex.resize(NewDepth, MexVector<uint32_t>(1, (uint32_t)0));
		// it is assumed that given that OldDepth == 0, Data is Empty
		return true;
	}
	else if (OldDepth < NewDepth) {
		MexVector<MexVector<uint32_t> > NewPartitionIndex(NewDepth, MexVector<uint32_t>(1, (uint32_t)0));

		if (!this->isempty()) {
			for (uint32_t i = 0; i < NewDepth - OldDepth - 1; ++i) {
				NewPartitionIndex[i].push_back(1);
			}
			NewPartitionIndex[NewDepth - OldDepth - 1].push_back(PartitionIndex[0].size() - 1);
		}
		
		for (uint32_t i = NewDepth - OldDepth; i < NewDepth; ++i) {
			NewPartitionIndex[i].swap(PartitionIndex[i - (NewDepth - OldDepth)]);
		}

		PartitionIndex.swap(NewPartitionIndex);
		return true;
	}
	else if (OldDepth > NewDepth) {
		if (NewDepth == 0) {
			if (this->isempty()) {
				PartitionIndex.clear();
				PartitionIndex.trim();
				return true;
			}
			else {
				return false;
			}
		}
		else {
			bool isValid = true;
			for (uint32_t i = 0; i < OldDepth - NewDepth; ++i)
				if (PartitionIndex[i].size() > 2) {
					isValid = false;
					break;
				}

			if (isValid) {
				MexVector<MexVector<uint32_t> > NewPartitionIndex(NewDepth);
				for (uint32_t i = 0; i < NewDepth; ++i)
					NewPartitionIndex[i].swap(PartitionIndex[i + OldDepth - NewDepth]);
				PartitionIndex.swap(NewPartitionIndex);
				return true;
			}
			else
				return false;
		}
	}
	else {
		// if NewDepth == OldDepth, do nothing, report success
		return true;
	}
}

template<typename T, class FVT_Al, class B>
inline void FlatVectTree<T, FVT_Al, B>::clear()
{
	uint32_t CurrDepth = this->depth();
	for (uint32_t i = 0; i < CurrDepth; ++i) {
		PartitionIndex[i].resize(1);
		PartitionIndex[i][0] = 0;
	}
	Data.clear();
}

template<typename T, class FVT_Al, class B>
inline void FlatVectTree<T, FVT_Al, B>::empty()
{
	this->clear();
	this->setDepth(0);
	this->PartitionIndex.trim();
	this->Data.trim();
}

/////////////////////////////////////////////////
// MEMORY RELEASE FUNCTIONS /////////////////////
/////////////////////////////////////////////////

template<typename T, class FVT_Al, class B>
inline void FlatVectTree<T, FVT_Al, B>::releaseMem(MexVector<MexVector<uint32_t, FVT_Al>, FVT_Al> &ReleasedPartInds, MexVector<T, FVT_Al> &ReleasedData)
{
	uint32_t TreeDepth = this->depth();

	// Relinquishing PartitionIndex and Data arrays 
	ReleasedPartInds.resize(TreeDepth);
	for (uint32_t i = 0; i < TreeDepth; ++i) {
		uint32_t NElemsinCurrLevel = PartitionIndex[i].size();
		ReleasedPartInds[i].assign(NElemsinCurrLevel, PartitionIndex[i].releaseArray());
	}
	uint32_t NElemsData = Data.size();
	ReleasedData.assign(NElemsData, Data.releaseArray());

	// Reinitializing PartitionInds and Data
	for (uint32_t i = 0; i < TreeDepth; ++i) {
		PartitionIndex[i].resize(1, 0);
	}
	// Data is already an empty (null) vector
}

/////////////////////////////////////////////////
// STATIC FUNCTIONS         /////////////////////
/////////////////////////////////////////////////

template<typename T, class FVT_Al, class B>
template<class AlSub, class Al, class AlData>
inline bool FlatVectTree<T, FVT_Al, B>::isValidFVT(const MexVector<MexVector<uint32_t, AlSub>, Al>& PartitionInds, const MexVector<T, AlData>& Data)
{
	bool isValid = true;
	
	size_t NLevels = PartitionInds.size();

	if (PartitionInds.size() > 0) {

		// Checking if sizes > 0
		for (uint32_t i = 0; i < NLevels && isValid; ++i) {
			size_t NElemsatCurrLevel = PartitionInds[i].size();
			if (PartitionInds[i].size() == 0) {
				isValid = false;
				break;
			}
		}

		// Checking sorted
		if(isValid)
		for (uint32_t i = 0; i < NLevels && isValid; ++i) {
			size_t NElemsatCurrLevel = PartitionInds[i].size();
			for (uint32_t j = 0; j < NElemsatCurrLevel - 1; ++j) {
				if (PartitionInds[i][j] > PartitionInds[i][j + 1]) {
					isValid = false;
					break;
				}
			}
		}

		// Chacking validity of first elements (must be 0)
		if (isValid)
		for (uint32_t i = 0; i < NLevels && isValid; ++i) {
			if (PartitionInds[i][0] != 0) {
				isValid = false;
				break;
			}
		}

		// Checking validity of BTE Elems
		if (isValid)
		for (uint32_t i = 0; i < NLevels - 1 && isValid; ++i) {
			if (PartitionInds[i].last() != PartitionInds[i + 1].size() - 1) {
				isValid = false;
				break;
			}
		}
		if (isValid)
		if (PartitionInds.last().last() != Data.size())
			isValid = false;
	}
	else {
		if (Data.size() > 0)
			isValid = false;
	}

	return isValid;
}
