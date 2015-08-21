#ifndef GENERIC_MEX_IO
#define GENERIC_MEX_IO

#include <matrix.h>
#include <mex.h>
#include <cstdarg>
#include <unordered_map>
#include <functional>
#include <cstdio>
#include "MexMem.hpp"
#include "LambdaToFunction.hpp"

typedef std::unordered_map<std::string, std::pair<void*, size_t> > StructArgTable;

template<typename T> inline mxArrayPtr assignmxArray(T &ScalarOut, mxClassID ClassID){

	mxArrayPtr ReturnPointer;
	if (is_arithmetic<T>::value){
		ReturnPointer = mxCreateNumericMatrix_730(1, 1, ClassID, mxREAL);
		*reinterpret_cast<T *>(mxGetData(ReturnPointer)) = ScalarOut;
	}
	else{
		ReturnPointer = mxCreateNumericMatrix_730(0, 0, ClassID, mxREAL);
	}

	return ReturnPointer;
}

template<typename T> inline mxArrayPtr assignmxArray(MexMatrix<T> &VectorOut, mxClassID ClassID){

	mxArrayPtr ReturnPointer = mxCreateNumericMatrix_730(0, 0, ClassID, mxREAL);
	if (VectorOut.ncols() && VectorOut.nrows()){
		mxSetM(ReturnPointer, VectorOut.ncols());
		mxSetN(ReturnPointer, VectorOut.nrows());
		mxSetData(ReturnPointer, VectorOut.releaseArray());
	}

	return ReturnPointer;
}

template<typename T> inline mxArrayPtr assignmxArray(MexVector<T> &VectorOut, mxClassID ClassID){

	mxArrayPtr ReturnPointer = mxCreateNumericMatrix_730(0, 0, ClassID, mxREAL);
	if (VectorOut.size()){
		mxSetM(ReturnPointer, VectorOut.size());
		mxSetN(ReturnPointer, 1);
		mxSetData(ReturnPointer, VectorOut.releaseArray());
	}
	return ReturnPointer;
}

template<typename T> inline mxArrayPtr assignmxArray(MexVector<MexVector<T> > &VectorOut, mxClassID ClassID){

	mxArrayPtr ReturnPointer;
	if (VectorOut.size()){
		ReturnPointer = mxCreateCellMatrix(VectorOut.size(), 1);

		size_t VectVectSize = VectorOut.size();
		for (int i = 0; i < VectVectSize; ++i){
			mxSetCell(ReturnPointer, i, assignmxArray(VectorOut[i], ClassID));
		}
	}
	else{
		ReturnPointer = mxCreateCellMatrix_730(0, 0);
	}
	return ReturnPointer;
}

inline void WriteOutput(char *Format, ...) {
	char buffertemp[256], bufferFinal[256];
	std::va_list Args;
	va_start(Args, Format);
	
	vsnprintf_s(buffertemp, 256, Format, Args);
	va_end(Args);

	char* tempIter = buffertemp;
	char* FinalIter = bufferFinal;
	for (; *tempIter != 0; ++tempIter, ++FinalIter){
		if (*tempIter == '%'){
			*FinalIter = '%';
			++FinalIter;
			*FinalIter = '%';
		}
		else{
			*FinalIter = *tempIter;
		}
	}
	*FinalIter = 0;
#ifdef MEX_LIB
	mexPrintf(bufferFinal);
	mexEvalString("drawnow();");
#elif defined MEX_EXE
#undef printf
	std::printf(bufferFinal);
	std::fflush(stdout);
#endif
}

inline void StringSplit(const char* InputString, const char* DelimString, MexVector<std::string> &SplitStringVect,
	bool includeBlanks = false){
	
	std::string tempInputString(InputString);
	SplitStringVect.resize(0);

	do{
		size_t DelimPos = tempInputString.find_first_of(DelimString);
		std::string currentSubString;
		if (DelimPos != std::string::npos){
			currentSubString = tempInputString.substr(0, DelimPos);
			tempInputString = tempInputString.substr(DelimPos + 1);
		}
		else{
			currentSubString = tempInputString;
			tempInputString = "";
		}

		if (includeBlanks || currentSubString != ""){
			SplitStringVect.push_back(currentSubString);
		}
	} while (tempInputString.length() != 0);
}

struct MexMemInputOps{
	bool IS_REQUIRED;
	bool NO_EXCEPT;
	bool QUIET;
	int  REQUIRED_SIZE;

	MexMemInputOps(){
		IS_REQUIRED = false;
		NO_EXCEPT = false;
		QUIET = false;
		REQUIRED_SIZE = -1;
	}

	MexMemInputOps(
		bool IS_REQUIRED_,
		int  REQUIRED_SIZE_ = -1,
		bool NO_EXCEPT_ = false,
		bool QUIET_ = false
		){
		IS_REQUIRED = IS_REQUIRED_;
		NO_EXCEPT = NO_EXCEPT_;
		QUIET = QUIET_;
		REQUIRED_SIZE = REQUIRED_SIZE_;
	}
};

inline MexMemInputOps getInputOps(int nOptions, va_list Options){
	MexMemInputOps InputOps;
	for (int i = 0; i < nOptions; ++i){
		char *CurrOption = va_arg(Options, char*);
		if (!_strcmpi("IS_REQUIRED", CurrOption)){
			InputOps.IS_REQUIRED = true;
		}
		else if (!_strcmpi("QUIET", CurrOption)){
			InputOps.QUIET = true;
		}
		else if (!_strcmpi("NO_EXCEPT", CurrOption)){
			InputOps.NO_EXCEPT = true;
		}
		else if (!_strcmpi("REQUIRED_SIZE", CurrOption)) {
			InputOps.REQUIRED_SIZE = va_arg(Options, int);
		}
	}
	return InputOps;
}

static mxArrayPtr getValidStructField(mxArrayPtr InputStruct, const char * FieldName, const MexMemInputOps & InputOps){
	
	// Processing Struct Name Heirarchy
	MexVector<std::string> NameHeirarchyVect;
	mxArrayPtr tempmxArrayPtr = InputStruct;
	StringSplit(FieldName, ".", NameHeirarchyVect);
	
	int NameHeirarchyDepth = NameHeirarchyVect.size();
	for (int i = 0; i < NameHeirarchyDepth - 1; ++i){
		tempmxArrayPtr = mxGetField(tempmxArrayPtr, 0, NameHeirarchyVect[i].data());
		if (tempmxArrayPtr == nullptr || mxIsEmpty(tempmxArrayPtr) || mxGetClassID(tempmxArrayPtr) != mxSTRUCT_CLASS){
			// If it is an invalid struct class
			if (InputOps.IS_REQUIRED){
				if (!InputOps.QUIET)
					WriteOutput("The required field '%s' is either empty or non-existant.\n", FieldName);
				if (!InputOps.NO_EXCEPT)
					throw ExOps::EXCEPTION_INVALID_INPUT;
			}
			return nullptr;
		}
	}
	
	// Extracting and Validating Final Vector
	tempmxArrayPtr = mxGetField(tempmxArrayPtr, 0, NameHeirarchyVect.last().data());
	if (tempmxArrayPtr != nullptr && !mxIsEmpty(tempmxArrayPtr)){
		size_t NumElems = mxGetNumberOfElements(tempmxArrayPtr);
		if (InputOps.REQUIRED_SIZE != -1 && InputOps.REQUIRED_SIZE != NumElems){
			if (!InputOps.QUIET)
				WriteOutput("The size of %s is required to be %d, it is currenty %d\n", FieldName, InputOps.REQUIRED_SIZE, NumElems);
			if (!InputOps.NO_EXCEPT)
				throw ExOps::EXCEPTION_INVALID_INPUT;
			return nullptr;
		}
		else{
			return tempmxArrayPtr;
		}
	}
	else if (InputOps.IS_REQUIRED){
		if (!InputOps.QUIET)
			WriteOutput("The required field '%s' is either empty or non-existant.\n", FieldName);
		if (!InputOps.NO_EXCEPT)
			throw ExOps::EXCEPTION_INVALID_INPUT;
		return nullptr;
	}
	return nullptr;
}

template<typename TypeRHS, typename TypeLHS>
inline typename MexVector<TypeLHS>::iterator MexTransform(
	typename MexVector<TypeRHS>::iterator RHSVectorBeg, 
	typename MexVector<TypeRHS>::iterator RHSVectorEnd,
	typename MexVector<TypeLHS>::iterator LHSVectorBeg,
	std::function<void(TypeLHS &, TypeRHS &)> transform_func){

	auto RHSIter = RHSVectorBeg;
	auto LHSIter = LHSVectorBeg;
	for (; RHSIter != RHSVectorEnd; ++RHSIter, ++LHSIter){
		transform_func(*LHSIter , *RHSIter);
	}
	return LHSIter;
}

template<typename TypeRHS, typename TypeLHS >
inline typename MexVector<TypeLHS>::iterator MexTransform(
	typename MexVector<TypeRHS>::iterator RHSVectorBeg, 
	typename MexVector<TypeRHS>::iterator RHSVectorEnd,
	typename MexVector<TypeLHS>::iterator LHSVectorBeg,
	typename std::function<TypeLHS(TypeRHS &)> transform_func){

	auto RHSIter = RHSVectorBeg;
	auto LHSIter = LHSVectorBeg;
	for (; RHSIter != RHSVectorEnd; ++RHSIter, ++LHSIter){
		*LHSIter = transform_func(*RHSIter);
	}
	return LHSIter;
}

template <typename T> inline void getInputfrommxArray(mxArray *InputArray, T &ScalarIn){
	if (InputArray != nullptr && !mxIsEmpty(InputArray))
		ScalarIn = *reinterpret_cast<T *>(mxGetData(InputArray));
}
template <typename TypeSrc, typename TypeDest> 
inline void getInputfrommxArray(mxArray *InputArray, TypeDest &ScalarIn){
	if (InputArray != nullptr && !mxIsEmpty(InputArray))
		ScalarIn = (TypeDest)(*reinterpret_cast<TypeSrc *>(mxGetData(InputArray)));
}
template <typename TypeSrc, typename TypeDest>
inline void getInputfrommxArray(mxArray *InputArray, TypeDest &ScalarIn,
	std::function<TypeDest(TypeSrc &)> casting_func){
	if (InputArray != nullptr && !mxIsEmpty(InputArray))
		ScalarIn = casting_func(*reinterpret_cast<TypeSrc *>(mxGetData(InputArray)));
}


template <typename T> inline void getInputfrommxArray(mxArray *InputArray, MexVector<T> &VectorIn){
	if (InputArray != nullptr && !mxIsEmpty(InputArray)){
		size_t NumElems = mxGetNumberOfElements(InputArray);
		T* tempDataPtr = reinterpret_cast<T *>(mxGetData(InputArray));
		VectorIn = MexVector<T>(NumElems);
		VectorIn.copyArray(0, tempDataPtr, NumElems);
	}
}

template <typename T> inline void getInputfrommxArray(mxArray *InputArray, MexVector<MexVector<T> > &VectorIn){
	if (InputArray != nullptr && !mxIsEmpty(InputArray) && mxGetClassID(InputArray) == mxCELL_CLASS){
		size_t NumElems = mxGetNumberOfElements(InputArray);
		mxArrayPtr* tempArrayPtr = reinterpret_cast<mxArrayPtr*>(mxGetData(InputArray));
		VectorIn = MexVector<MexVector<T> >(NumElems, MexVector<T>(0));
		for (int i = 0; i < NumElems; ++i){
			getInputfrommxArray(tempArrayPtr[i], VectorIn[i]);
		}
	}
}

template <typename TypeSrc, typename TypeDest> 
inline void getInputfrommxArray(
	mxArray *InputArray, 
	MexVector<TypeDest> &VectorIn, 
	void (*casting_fun)(TypeSrc &SrcElem, TypeDest &DestElem)){
	
	if (InputArray != nullptr && !mxIsEmpty(InputArray)){
		size_t NumElems = mxGetNumberOfElements(InputArray);
		TypeSrc* tempArrayPtr = reinterpret_cast<TypeSrc*>(mxGetData(InputArray));
		VectorIn.resize(NumElems);
		for (int i = 0; i < NumElems; ++i){
			casting_fun(tempArrayPtr[i], VectorIn[i]);
		}
	}
}

template <typename TypeSrc, typename TypeDest>
inline void getInputfrommxArray(
	mxArray *InputArray,
	MexVector<TypeDest> &VectorIn,
	std::function<void(TypeSrc &, TypeDest &)> casting_fun){

	if (InputArray != nullptr && !mxIsEmpty(InputArray)){
		size_t NumElems = mxGetNumberOfElements(InputArray);
		TypeSrc* tempArrayPtr = reinterpret_cast<TypeSrc*>(mxGetData(InputArray));
		VectorIn.resize(NumElems);
		for (int i = 0; i < NumElems; ++i){
			casting_fun(tempArrayPtr[i], VectorIn[i]);
		}
	}
}

template <typename TypeSrc, typename TypeDest>
inline void getInputfrommxArray(
	mxArray *InputArray,
	MexVector<TypeDest> &VectorIn){

	if (InputArray != nullptr && !mxIsEmpty(InputArray)){
		size_t NumElems = mxGetNumberOfElements(InputArray);
		TypeSrc* tempArrayPtr = reinterpret_cast<TypeSrc*>(mxGetData(InputArray));
		VectorIn.resize(NumElems); // This will not erase old data
		for (int i = 0; i < NumElems; ++i){
			tempArrayPtr[i] = (TypeDest)VectorIn[i];
		}
	}
}

// Scalar Input
template <typename T> inline int getInputfromStruct(mxArray *InputStruct, const char* FieldName, NonDeduc(T) &ScalarIn, int nOptions = 0, ...){

	// Getting Input Options
	MexMemInputOps InputOps;

	va_list OptionList;
	va_start(OptionList, nOptions);
	InputOps = getInputOps(nOptions, OptionList);
	va_end(OptionList);

	InputOps.REQUIRED_SIZE = -1;
	mxArrayPtr StructFieldPtr = getValidStructField(InputStruct, FieldName, InputOps);

	if (StructFieldPtr != nullptr){
		getInputfrommxArray(StructFieldPtr, ScalarIn);
		return 0;
	}
	else{
		return 1;
	}
}

// Scalar Input (Type Casting)
template <typename TypeSrc, typename TypeDest> 
inline int getInputfromStruct(mxArray *InputStruct, const char* FieldName, TypeDest &ScalarIn, int nOptions = 0, ...){

	// Getting Input Options
	MexMemInputOps InputOps;

	va_list OptionList;
	va_start(OptionList, nOptions);
	InputOps = getInputOps(nOptions, OptionList);
	va_end(OptionList);

	InputOps.REQUIRED_SIZE = -1;
	mxArrayPtr StructFieldPtr = getValidStructField(InputStruct, FieldName, InputOps);

	if (StructFieldPtr != nullptr){
		getInputfrommxArray<TypeSrc>(StructFieldPtr, ScalarIn);
		return 0;
	}
	else{
		return 1;
	}
}

// MexVector<T>
template <typename T> inline int getInputfromStruct(mxArray *InputStruct, const char* FieldName, MexVector<NonDeduc(T)> &VectorIn, int nOptions = 0, ...){
	
	// processing options for input
	MexMemInputOps InputOps;

	va_list OptionArray;
	va_start(OptionArray, nOptions);
	InputOps = getInputOps(nOptions, OptionArray);
	va_end(OptionArray);

	// Processing Data
	mxArrayPtr StructFieldPtr = getValidStructField(InputStruct, FieldName, InputOps);
	if (StructFieldPtr != nullptr){
		getInputfrommxArray(StructFieldPtr, VectorIn);
		return 0;
	}
	else{
		return 1;
	}
}

// MexVector<MexVector<T> >
// This code is completely the same as the earlier one Except for 
// additional type checking, as the part that is different uses the 
// templated call to getInputfrommxArray making all the code identical
template <typename T> inline int getInputfromStruct(mxArray *InputStruct, const char* FieldName, MexVector<MexVector<NonDeduc(T)> > &VectorIn, int nOptions = 0, ...){

	// processing options for input
	MexMemInputOps InputOps;

	va_list OptionArray;
	va_start(OptionArray, nOptions);
	InputOps = getInputOps(nOptions, OptionArray);
	va_end(OptionArray);

	// Processing Data
	mxArrayPtr StructFieldPtr = getValidStructField(InputStruct, FieldName, InputOps);
	if (StructFieldPtr != nullptr && mxGetClassID(StructFieldPtr) == mxCELL_CLASS){
		getInputfrommxArray(StructFieldPtr, VectorIn);
		return 0;
	}
	else{
		return 1;
	}
}

// Taking structure as input
template <typename T>
inline int getInputfromStruct(
	mxArray *InputStruct, const char* FieldName, 
	MexVector<T> &VectorIn, 
	void(*struct_inp_fun)(StructArgTable &ArgumentVects, T &DestElem),
	int nOptions = 0, ...){

	// processing options for input
	MexMemInputOps InputOps;

	va_list OptionArray;
	va_start(OptionArray, nOptions);
	InputOps = getInputOps(nOptions, OptionArray);
	va_end(OptionArray);

	// Processing Data
	// converting Field Name into array of field names by splitting at '-'
	MexVector<std::string> FieldNamesVect(0);
	StructArgTable StructFieldmxArrays;

	// Split the string into its components by delimeters ' -/,'
	StringSplit(FieldName, " -/,", FieldNamesVect);

	// For each valid field name stored in vector, add entry in map
	size_t PrevNumElems = 0; // used to enforce condition that all are of equal size

	for (int i = 0; i < FieldNamesVect.size(); ++i){

		mxArrayPtr StructFieldPtr;

		// Size constraints are imposed based on wether 
		// a) A prerequisite size has been specified
		// b) We have another reference size to impose size equality
		// c) We have nothing (in which case no size constraints)
		if (InputOps.REQUIRED_SIZE != -1){
			 StructFieldPtr = getValidStructField(InputStruct, FieldNamesVect[i].data(), InputOps);
		}
		else if(PrevNumElems != 0){
			MexMemInputOps tempInputOps = InputOps;
			tempInputOps.REQUIRED_SIZE = PrevNumElems;
			StructFieldPtr = getValidStructField(InputStruct, FieldNamesVect[i].data(), tempInputOps);
		}
		else{
			MexMemInputOps tempInputOps = InputOps;
			StructFieldPtr = getValidStructField(InputStruct, FieldNamesVect[i].data(), tempInputOps);
		}

		// Storing the element conditionally depending on whether or not it exists
		size_t NumElems = (StructFieldPtr != nullptr) ? mxGetNumberOfElements(StructFieldPtr) : 0;
		if (StructFieldPtr != NULL){
			// Store the pointer and data size for current field
			int ElemSize = mxGetElementSize(StructFieldPtr);
			StructFieldmxArrays.insert(
				std::pair<std::string, pair<void*, size_t> >(
					FieldNamesVect[i], 
					pair<void*, size_t>(mxGetData(StructFieldPtr), ElemSize)
				));
			PrevNumElems = NumElems;
		}
		else{
			// Store the null pointer and 0 size for current field
			StructFieldmxArrays.insert(
				std::pair<std::string, pair<void*, size_t> >(
					FieldNamesVect[i],
					pair<void*, size_t>(nullptr, 0);
				));
		}
	}
	// At this point PrevNumElems represents the number of elements in the arrays listed
	VectorIn.resize(PrevNumElems);
	for (int i = 0; i < PrevNumElems; ++i){
		struct_inp_fun(StructFieldmxArrays, VectorIn[i]);
		// updating pointers to the next element
		for (auto Elem : StructFieldmxArrays){
			auto &CurrElem = StructFieldmxArrays[Elem.first];
			if (CurrElem.first != nullptr)
				CurrElem.first = (char *)CurrElem.first + CurrElem.second;
		}
	}
	return 0;
}

// Taking structure as input (functors)
template <typename T>
inline int getInputfromStruct(
	mxArray *InputStruct, const char* FieldName,
	MexVector<T> &VectorIn,
	std::function<void(StructArgTable &ArgumentVects, T &DestElem)> struct_inp_fun,
	int nOptions = 0, ...){

	// processing options for input
	MexMemInputOps InputOps;

	va_list OptionArray;
	va_start(OptionArray, nOptions);
	InputOps = getInputOps(nOptions, OptionArray);
	va_end(OptionArray);

	// Processing Data
	// converting Field Name into array of field names by splitting at '-'
	MexVector<std::string> FieldNamesVect(0);
	StructArgTable StructFieldmxArrays;

	// Split the string into its components by delimeters ' -/,'
	StringSplit(FieldName, " -/,", FieldNamesVect);

	// For each valid field name stored in vector, add entry in map
	size_t PrevNumElems = 0; // used to enforce condition that all are of equal size

	for (int i = 0; i < FieldNamesVect.size(); ++i){

		mxArrayPtr StructFieldPtr;

		// Size constraints are imposed based on wether 
		// a) A prerequisite size has been specified
		// b) We have another reference size to impose size equality
		// c) We have nothing (in which case no size constraints)
		if (InputOps.REQUIRED_SIZE != -1){
			StructFieldPtr = getValidStructField(InputStruct, FieldNamesVect[i].data(), InputOps);
		}
		else if (PrevNumElems != 0){
			MexMemInputOps tempInputOps = InputOps;
			tempInputOps.REQUIRED_SIZE = PrevNumElems;
			StructFieldPtr = getValidStructField(InputStruct, FieldNamesVect[i].data(), tempInputOps);
		}
		else{
			MexMemInputOps tempInputOps = InputOps;
			StructFieldPtr = getValidStructField(InputStruct, FieldNamesVect[i].data(), tempInputOps);
		}

		// Storing the element conditionally depending on whether or not it exists
		size_t NumElems = (StructFieldPtr != nullptr) ? mxGetNumberOfElements(StructFieldPtr) : 0;
		if (StructFieldPtr != NULL){
			// Store the pointer and data size for current field
			int ElemSize = mxGetElementSize(StructFieldPtr);
			StructFieldmxArrays.insert(
				std::pair<std::string, pair<void*, size_t> >(
				FieldNamesVect[i],
				pair<void*, size_t>(mxGetData(StructFieldPtr), ElemSize)
				));
			PrevNumElems = NumElems;
		}
		else{
			// Store the null pointer and 0 size for current field
			StructFieldmxArrays.insert(
				std::pair<std::string, pair<void*, size_t> >(
				FieldNamesVect[i],
				pair<void*, size_t>(nullptr, 0)
			));
		}
	}
	// At this point PrevNumElems represents the number of elements in the arrays listed
	VectorIn.resize(PrevNumElems); // Does not Delete Previous Elements
	for (int i = 0; i < PrevNumElems; ++i){
		struct_inp_fun(StructFieldmxArrays, VectorIn[i]);
		// updating pointers to the next element
		for (auto Elem : StructFieldmxArrays){
			auto &CurrElem = StructFieldmxArrays[Elem.first];
			if (CurrElem.first != nullptr)
				CurrElem.first = (char *)CurrElem.first + CurrElem.second;
		}
	}
	return 0;
}

// Type Casting included (implicit)

template <typename TypeSrc, typename TypeDest>
inline int getInputfromStruct(
	mxArray *InputStruct,
	const char* FieldName,
	MexVector<TypeDest> &VectorIn,
	int nOptions = 0, ...){

	// processing options for input
	MexMemInputOps InputOps;

	va_list OptionArray;
	va_start(OptionArray, nOptions);
	InputOps = getInputOps(nOptions, OptionArray);
	va_end(OptionArray);

	// Processing Data
	mxArrayPtr StructFieldPtr = getValidStructField(InputStruct, FieldName, InputOps);
	if (StructFieldPtr != nullptr){
		getInputfrommxArray<TypeSrc, TypeDest>(tempmxArrayPtr, VectorIn);
		return 0;
	}
	else{
		return 1;
	}
}


// Type Casting included (explicit)
template <typename TypeSrc, typename TypeDest>
inline int getInputfromStruct(
	mxArray *InputStruct,
	const char* FieldName,
	MexVector<TypeDest> &VectorIn,
	void(*casting_fun)(TypeSrc &SrcElem, TypeDest &DestElem),
	int nOptions = 0, ...){

	// processing options for input
	MexMemInputOps InputOps;

	va_list OptionArray;
	va_start(OptionArray, nOptions);
	InputOps = getInputOps(nOptions, OptionArray);
	va_end(OptionArray);

	// Processing Data
	mxArrayPtr StructFieldPtr = getValidStructField(InputStruct, FieldName, InputOps);
	if (StructFieldPtr != nullptr){
		getInputfrommxArray<TypeSrc, TypeDest>(tempmxArrayPtr, VectorIn, casting_fun);
		return 0;
	}
	else{
		return 1;
	}
}

// Type Casting included (explicit) (functors)
template <typename TypeSrc, typename TypeDest>
inline int getInputfromStruct(
	mxArray *InputStruct,
	const char* FieldName,
	MexVector<TypeDest> &VectorIn,
	std::function<void(TypeSrc &, TypeDest &)> casting_fun,
	int nOptions = 0, ...){

	// processing options for input
	MexMemInputOps InputOps;

	va_list OptionArray;
	va_start(OptionArray, nOptions);
	InputOps = getInputOps(nOptions, OptionArray);
	va_end(OptionArray);

	// Processing Data
	mxArrayPtr StructFieldPtr = getValidStructField(InputStruct, FieldName, InputOps);
	if (StructFieldPtr != nullptr){
		getInputfrommxArray<TypeSrc, TypeDest>(tempmxArrayPtr, VectorIn, casting_fun);
		return 0;
	}
	else{
		return 1;
	}
}
#endif