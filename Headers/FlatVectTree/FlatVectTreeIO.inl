#include "FlatVectTree.hpp"

#define VECT_TREE_FIELD_INFO_(T) FieldInfo< T, typename std::enable_if<isFlatVectTree< T >::value>::type>::

template <typename T>
inline bool VECT_TREE_FIELD_INFO_(T) CheckType(mxArrayPtr InputmxArray) {
	bool isValid = true;
	if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray) && mxIsStruct(InputmxArray)) {
		// Check if ClassName Field matches 'FlatCellArray'
		mxArrayPtr ClassNamemxArr = getValidStructField<char16_t>(InputmxArray, "ClassName");
		if (ClassNamemxArr != nullptr) {
			char* ClassNameStr = mxArrayToString(ClassNamemxArr);
			if (!std::strcmp(ClassNameStr, "FlatCellArray")) {
				// Extract elements from PartitionIndex and Data and confirm if 
				// they represent a valid FlatVectTree / FlatCellArray
				typedef typename isFlatVectTree<T>::type TypeofData;
				
				MexVector<MexVector<uint32_t> > PartitionIndex;
				MexVector<TypeofData> Data;

				moveIntoVectors(InputmxArray, PartitionIndex, Data);

				// Validate FlatVectTree
				isValid = FlatVectTree<TypeofData>::isValidFVT(PartitionIndex, Data);
			}
			else
				isValid = false;
			mxFree(ClassNameStr);
		}
		else
			isValid = false;
	}
	else if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray))
		isValid = false;
	else
		isValid = true;

	return isValid;
}

template <typename T>
inline uint32_t VECT_TREE_FIELD_INFO_(T) getSize(mxArrayPtr InputmxArray) {
	
	// This function performs no validation of the data except for preventing 
	// read of nullptr

	size_t NumElems = 0;

	// If array is non-empty, calculate size
	if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray) && mxIsStruct(InputmxArray)) {
		// Extract size from Top level of PartitionIndex 
		mxArrayPtr PartitionIndexmxArr = mxGetField(InputmxArray, 0, "PartitionIndex");
		if (PartitionIndexmxArr != nullptr && !mxIsEmpty(PartitionIndexmxArr)) {
			mxArrayPtr PartitionIndexTopLevel = mxGetCell(PartitionIndexmxArr, 0);
			if (PartitionIndexTopLevel != nullptr) {
				uint32_t temp = mxGetNumberOfElements(PartitionIndexTopLevel);
				NumElems = (temp > 0) ? temp - 1 : 0;
			}
		}
	}
	return NumElems;
}

template<typename T>
inline uint32_t VECT_TREE_FIELD_INFO_(T) getDepth(mxArrayPtr InputmxArray)
{
	// This function performs no validation of the data except for preventing 
	// read of nullptr

	size_t TreeDepth = 0;

	// If array is non-empty, calculate depth
	if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray) && mxIsStruct(InputmxArray)) {
		// Extract depth from PartitionIndex 
		mxArrayPtr PartitionIndexmxArr = mxGetField(InputmxArray, 0, "PartitionIndex");
		if (PartitionIndexmxArr != nullptr && !mxIsEmpty(PartitionIndexmxArr)) {
			TreeDepth = mxGetNumberOfElements(PartitionIndexmxArr);
		}
	}
	return TreeDepth;
}

template<typename T>
inline void VECT_TREE_FIELD_INFO_(T) moveIntoVectors(
	mxArrayPtr InputmxArray, 
	MexVector<MexVector<uint32_t> >& PartitionIndexIn, 
	MexVector<typename isFlatVectTree<T>::type>& DataIn)
{
	// This function moves the data in InputmxArray (interpreted as a FlatCellArray)
	// into the given PartitionIndexIn and DataIn. If the PartitionIndex and Data
	// fields are of invalid type then it raises an exception. If they are empty, then
	// he corresponding vector is empty. The function makes no attempt to verify
	// the correctness of the data.

	// Getting and type validating the fields "PartitionIndexIn" and "Data"
	typedef typename isFlatVectTree<T>::type TypeofData;
	mxArrayPtr PartitionIndexmxArr = getValidStructField<MexVector<MexVector<uint32_t> > >(InputmxArray, "PartitionIndex");
	mxArrayPtr DatamxArr = getValidStructField<TypeofData>(InputmxArray, "Data");

	// Calculating TreeDepth
	uint32_t TreeDepth = 0;
	if (PartitionIndexmxArr)
		TreeDepth = mxGetNumberOfElements(PartitionIndexmxArr);
	
	// Initializing Input Vectors
	PartitionIndexIn.resize(0);
	DataIn.resize(0);

	// Filling PartitionIndexIn
	if (TreeDepth > 0) {
		PartitionIndexIn.resize(TreeDepth, MexVector<uint32_t>());
		mxArrayPtr * PartitionIndexLevelsmxArr = (mxArrayPtr *)mxGetData(PartitionIndexmxArr);
		for (int i = 0; i < TreeDepth; ++i) {
			int LevelNElems = FieldInfo<MexVector<uint32_t> >::getSize(PartitionIndexLevelsmxArr[i]);
			if (LevelNElems > 0) {
				PartitionIndexIn[i].assign(LevelNElems, (uint32_t *)mxGetData(PartitionIndexLevelsmxArr[i]), false);
			}
		}
	}

	// Filling DataIn
	if (DatamxArr != nullptr)
		DataIn.assign(mxGetNumberOfElements(DatamxArr), (TypeofData *)mxGetData(DatamxArr), false);
}

template<typename T> inline mxArrayPtr assignmxArray(FlatVectTree<T> &FlatVectTreeOut) {

	const char* FieldNames[] = {
		"ClassName",
		"PartitionIndex",
		"Data"
	};
	size_t NFields = 3;
	mwSize ArraySize[] = { 1, 1 };

	mxArrayPtr ReturnPtr = mxCreateStructArray(2, ArraySize, 3, FieldNames);
	
	// Releasing Memory of FlatVectTreeOut
	MexVector<MexVector<uint32_t> > PartitionIndex;
	MexVector<T> Data;
	FlatVectTreeOut.releaseMem(PartitionIndex, Data);

	mxSetField(ReturnPtr, 0, "ClassName"     , mxCreateString("FlatCellArray"));
	mxSetField(ReturnPtr, 0, "PartitionIndex", assignmxArray(PartitionIndex, mxUINT32_CLASS));
	mxSetField(ReturnPtr, 0, "Data"          , assignmxArray(Data          , GetMexType<T>::typeVal));

	return ReturnPtr;
}

template <typename T, class Al> static void getInputfrommxArray(mxArray *InputArray, FlatVectTree<T, Al> &FlatVectTreeIn) {

	// Note: in this function, it is assumed that InputArray is a Valid FlatVectTreeIn
	// with non-zero depth.

	// Declared with mxAllocator as they are to be used to move the 
	// input mex arrays
	MexVector<MexVector<uint32_t> > PartitionIndex;
	MexVector<T> Data;

	FieldInfo<FlatVectTree<T, Al> >::moveIntoVectors(InputArray, PartitionIndex, Data);
	FlatVectTreeIn.assign(PartitionIndex, Data);
}

template <typename T, class Al> static int getInputfromStruct(mxArray *InputStruct, const char* FieldName, uint32_t RequiredDepth, FlatVectTree<NonDeduc(T), Al> &FlatVectTreeIn, int nOptions, ...) {
	// Getting Input Options
	MexMemInputOps InputOps;

	va_list OptionList;
	va_start(OptionList, nOptions);
	InputOps = getInputOps(nOptions, OptionList);
	va_end(OptionList);

	mxArrayPtr StructFieldPtr = getValidStructField<FlatVectTree<T,Al> >(InputStruct, FieldName, InputOps);
	if (StructFieldPtr != nullptr) {
		uint32_t GivenTreeDepth = FieldInfo<FlatVectTree<T, Al> >::getDepth(StructFieldPtr);
		if (GivenTreeDepth > 0 && GivenTreeDepth != RequiredDepth) {
			if (!InputOps.QUIET)
				WriteOutput("The depth of the given FlatVectTree (%d), does not match the depth required (%d)\n", GivenTreeDepth, RequiredDepth);
			if (!InputOps.NO_EXCEPT)
				throw ExOps::EXCEPTION_INVALID_INPUT;
		}
		else if (GivenTreeDepth > 0)
			getInputfrommxArray(StructFieldPtr, FlatVectTreeIn);
		else
			return 1;
		return 0;
	}
	else {
		return 1;
	}
}