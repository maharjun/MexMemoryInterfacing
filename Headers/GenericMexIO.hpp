#ifndef GENERIC_MEX_IO
#define GENERIC_MEX_IO

#include <matrix.h>
#include <mex.h>
#include <cstdarg>
#include <unordered_map>
#include <functional>
#include "MexMem.hpp"

typedef std::unordered_map<string, pair<void*, size_t> > StructArgTable;

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

template <typename T> inline void getInputfrommxArray(mxArray *InputArray, T &ScalarIn){
	if (InputArray != NULL && !mxIsEmpty(InputArray))
		ScalarIn = *reinterpret_cast<T *>(mxGetData(InputArray));
}
template <typename TypeSrc, typename TypeDest> 
inline void getInputfrommxArray(mxArray *InputArray, TypeDest &ScalarIn){
	if (InputArray != NULL && !mxIsEmpty(InputArray))
		ScalarIn = (TypeDest)(*reinterpret_cast<TypeSrc *>(mxGetData(InputArray)));
}
template <typename TypeSrc, typename TypeDest>
inline void getInputfrommxArray(mxArray *InputArray, TypeDest &ScalarIn,
	std::function<TypeDest(TypeSrc &)> casting_func){
	if (InputArray != NULL && !mxIsEmpty(InputArray))
		ScalarIn = casting_func(*reinterpret_cast<TypeSrc *>(mxGetData(InputArray)));
}


template <typename T> inline void getInputfrommxArray(mxArray *InputArray, MexVector<T> &VectorIn){
	if (InputArray != NULL && !mxIsEmpty(InputArray)){
		size_t NumElems = mxGetNumberOfElements(InputArray);
		T* tempDataPtr = reinterpret_cast<T *>(mxGetData(InputArray));
		VectorIn = MexVector<T>(NumElems);
		VectorIn.copyArray(0, tempDataPtr, NumElems);
	}
}

template <typename T> inline void getInputfrommxArray(mxArray *InputArray, MexVector<MexVector<T> > &VectorIn){
	if (InputArray != NULL && !mxIsEmpty(InputArray) && mxGetClassID(InputArray) == mxCELL_CLASS){
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
	
	if (InputArray != NULL && !mxIsEmpty(InputArray)){
		size_t NumElems = mxGetNumberOfElements(InputArray);
		TypeSrc* tempArrayPtr = reinterpret_cast<TypeSrc*>(mxGetData(InputArray));
		VectorIn = MexVector<TypeDest>(NumElems);
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

	if (InputArray != NULL && !mxIsEmpty(InputArray)){
		size_t NumElems = mxGetNumberOfElements(InputArray);
		TypeSrc* tempArrayPtr = reinterpret_cast<TypeSrc*>(mxGetData(InputArray));
		VectorIn = MexVector<TypeDest>(NumElems);
		for (int i = 0; i < NumElems; ++i){
			casting_fun(tempArrayPtr[i], VectorIn[i]);
		}
	}
}

template <typename TypeSrc, typename TypeDest>
inline void getInputfrommxArray(
	mxArray *InputArray,
	MexVector<TypeDest> &VectorIn){

	if (InputArray != NULL && !mxIsEmpty(InputArray)){
		size_t NumElems = mxGetNumberOfElements(InputArray);
		TypeSrc* tempArrayPtr = reinterpret_cast<TypeSrc*>(mxGetData(InputArray));
		VectorIn = MexVector<TypeDest>(NumElems);
		for (int i = 0; i < NumElems; ++i){
			tempArrayPtr[i] = (TypeDest)VectorIn[i];
		}
	}
}

// Scalar Input
template <typename T> inline int getInputfromStruct(mxArray *InputStruct, const char* FieldName, T &ScalarIn, int nOptions = 0, ...){

	// Getting Input Options
	MexMemInputOps InputOps;

	va_list OptionList;
	va_start(OptionList, nOptions);
	getInputOps(nOptions, OptionList);
	va_end(OptionList);

	mxArray* tempmxArrayPtr = mxGetField(InputStruct, 0, FieldName);

	if (tempmxArrayPtr != NULL && !mxIsEmpty(tempmxArrayPtr))
		getInputfrommxArray(tempmxArrayPtr, ScalarIn);
	else if (InputOps.IS_REQUIRED){
		if (!InputOps.QUIET)
			WriteOutput("The field '%s' is either empty or non-existant", FieldName);
		if (!InputOps.NO_EXCEPT)
			throw ExOps::EXCEPTION_INVALID_INPUT;
		return 1;
	}
	return 0;
}

// Scalar Input (Type Casting)
template <typename TypeSrc, typename TypeDest> 
inline int getInputfromStruct(mxArray *InputStruct, const char* FieldName, TypeDest &ScalarIn, int nOptions = 0, ...){

	// Getting Input Options
	MexMemInputOps InputOps;

	va_list OptionList;
	va_start(OptionList, nOptions);
	getInputOps(nOptions, OptionList);
	va_end(OptionList);

	mxArray* tempmxArrayPtr = mxGetField(InputStruct, 0, FieldName);
	if (tempmxArrayPtr != NULL && !mxIsEmpty(tempmxArrayPtr))
		getInputfrommxArray<TypeSrc>(tempmxArrayPtr, ScalarIn);
	else if (InputOps.IS_REQUIRED){
		if (!InputOps.QUIET)
			WriteOutput("The field '%s' is either empty or non-existant", FieldName);
		if (!InputOps.NO_EXCEPT)
			throw ExOps::EXCEPTION_INVALID_INPUT;
		return 1;
	}
	return 0;
}
// MexVector<T>
template <typename T> inline int getInputfromStruct(mxArray *InputStruct, const char* FieldName, MexVector<T> &VectorIn, int nOptions = 0, ...){
	
	// processing options for input
	MexMemInputOps InputOps;

	va_list OptionArray;
	va_start(OptionArray, nOptions);
	InputOps = getInputOps(nOptions, OptionArray);
	va_end(OptionArray);

	// Processing Data
	mxArray* tempmxArrayPtr = mxGetField(InputStruct, 0, FieldName);
	if (tempmxArrayPtr != NULL && !mxIsEmpty(tempmxArrayPtr)){
		size_t NumElems = mxGetNumberOfElements(tempmxArrayPtr);
		if (InputOps.REQUIRED_SIZE != -1 && InputOps.REQUIRED_SIZE != NumElems){
			if (!InputOps.QUIET)
				WriteOutput("The size of %s is required to be %d, it is currenty %d\n", FieldName, InputOps.REQUIRED_SIZE, NumElems);
			if (!InputOps.NO_EXCEPT)
				throw ExOps::EXCEPTION_INVALID_INPUT;
			return 1; 
		}
		else{
			getInputfrommxArray(tempmxArrayPtr, VectorIn);
		}
	}
	else if(InputOps.IS_REQUIRED){
		if (!InputOps.QUIET)
			WriteOutput("The required field '%s' is either empty or non-existant.\n", FieldName);
		if (!InputOps.NO_EXCEPT)
			throw ExOps::EXCEPTION_INVALID_INPUT;
		return 1;
	}
	return 0;
}

// MexVector<MexVector<T> >
// This code is completely the same as the earlier one as the part that is different
// uses the templated call to getInputfrommxArray making all the code identical

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
	std::string FieldNameString(FieldName);
	MexVector<std::string> FieldNamesVect(0);
	StructArgTable StructFieldmxArrays;
	// Split the string into its components by delimeters ' -/,'
	do{
		int StrLen = FieldNameString.length();
		size_t DelimPos = FieldNameString.find_last_of(" -/,");
		string currentField;
		if (DelimPos != std::string::npos){
			currentField = FieldNameString.substr(DelimPos+1);
			FieldNameString.resize(DelimPos);
		}
		else{
			currentField = FieldNameString;
			FieldNameString = "";
		}
		if (currentField != ""){
			FieldNamesVect.push_back(currentField);
		}
	} while (FieldNameString.length() != 0);

	// For each valid field name stored in vector, add entry in map

	size_t PrevNumElems = 0; // used to enforce condition that all are of equal size

	for (int i = 0; i < FieldNamesVect.size(); ++i){
		mxArray* tempmxArrayPtr = mxGetField(InputStruct, 0, FieldNamesVect[i].data());
		size_t NumElems = (tempmxArrayPtr != NULL)?mxGetNumberOfElements(tempmxArrayPtr):0;

		if (NumElems != 0){
			if (PrevNumElems != 0 && PrevNumElems != NumElems){
				// Note that the PrevNumElems != 0 precludes any issues with the i-1 below
				if (!InputOps.QUIET)
					WriteOutput("The no. of elems of Field %s (%d) is not consistent with Field %s (%d)\n", 
						FieldNamesVect[i].data(), NumElems,
						FieldNamesVect[i-1].data(), PrevNumElems);
				if (!InputOps.NO_EXCEPT)
					throw ExOps::EXCEPTION_INVALID_INPUT;
				return 1;
			}
			else if (InputOps.REQUIRED_SIZE != -1 && InputOps.REQUIRED_SIZE != NumElems){
				if (!InputOps.QUIET)
					WriteOutput("The size of %s is required to be %d, it is currenty %d\n", FieldName, InputOps.REQUIRED_SIZE, NumElems);
				if (!InputOps.NO_EXCEPT)
					throw ExOps::EXCEPTION_INVALID_INPUT;
				return 1;
			}
			else{
				int ElemSize = mxGetElementSize(tempmxArrayPtr);
				StructFieldmxArrays.insert(
					std::pair<std::string, pair<void*, size_t> >(
						FieldNamesVect[i], 
						pair<void*, size_t>(mxGetData(tempmxArrayPtr), ElemSize)
					));
				PrevNumElems = NumElems;
			}
		}
		else if (InputOps.IS_REQUIRED){
			if (!InputOps.QUIET)
				WriteOutput("The required field '%s' is either empty or non-existant.\n", FieldName);
			if (!InputOps.NO_EXCEPT)
				throw ExOps::EXCEPTION_INVALID_INPUT;
			return 1;
		}
	}
	// At this point PrevNumElems represents the number of elements in the arrays listed
	VectorIn = MexVector<T>(PrevNumElems);
	for (int i = 0; i < PrevNumElems; ++i){
		struct_inp_fun(StructFieldmxArrays, VectorIn[i]);
		// updating pointers to the next element
		for (auto Elem : StructFieldmxArrays){
			auto &CurrElem = StructFieldmxArrays[Elem.first];
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
	std::string FieldNameString(FieldName);
	MexVector<std::string> FieldNamesVect(0);
	StructArgTable StructFieldmxArrays;
	// Split the string into its components by delimeters ' -/,'
	do{
		int StrLen = FieldNameString.length();
		size_t DelimPos = FieldNameString.find_last_of(" -/,");
		string currentField;
		if (DelimPos != std::string::npos){
			currentField = FieldNameString.substr(DelimPos + 1);
			FieldNameString.resize(DelimPos);
		}
		else{
			currentField = FieldNameString;
			FieldNameString = "";
		}
		if (currentField != ""){
			FieldNamesVect.push_back(currentField);
		}
	} while (FieldNameString.length() != 0);

	// For each valid field name stored in vector, add entry in map

	size_t PrevNumElems = 0; // used to enforce condition that all are of equal size

	for (int i = 0; i < FieldNamesVect.size(); ++i){
		mxArray* tempmxArrayPtr = mxGetField(InputStruct, 0, FieldNamesVect[i].data());
		size_t NumElems = (tempmxArrayPtr != NULL) ? mxGetNumberOfElements(tempmxArrayPtr) : 0;

		if (NumElems != 0){
			if (PrevNumElems != 0 && PrevNumElems != NumElems){
				// Note that the PrevNumElems != 0 precludes any issues with the i-1 below
				if (!InputOps.QUIET)
					WriteOutput("The no. of elems of Field %s (%d) is not consistent with Field %s (%d)\n",
					FieldNamesVect[i].data(), NumElems,
					FieldNamesVect[i - 1].data(), PrevNumElems);
				if (!InputOps.NO_EXCEPT)
					throw ExOps::EXCEPTION_INVALID_INPUT;
				return 1;
			}
			else if (InputOps.REQUIRED_SIZE != -1 && InputOps.REQUIRED_SIZE != NumElems){
				if (!InputOps.QUIET)
					WriteOutput("The size of %s is required to be %d, it is currenty %d\n", FieldName, InputOps.REQUIRED_SIZE, NumElems);
				if (!InputOps.NO_EXCEPT)
					throw ExOps::EXCEPTION_INVALID_INPUT;
				return 1;
			}
			else{
				int ElemSize = mxGetElementSize(tempmxArrayPtr);
				StructFieldmxArrays.insert(
					std::pair<std::string, pair<void*, size_t> >(
					FieldNamesVect[i],
					pair<void*, size_t>(mxGetData(tempmxArrayPtr), ElemSize)
					));
				PrevNumElems = NumElems;
			}
		}
		else if (InputOps.IS_REQUIRED){
			if (!InputOps.QUIET)
				WriteOutput("The required field '%s' is either empty or non-existant.\n", FieldName);
			if (!InputOps.NO_EXCEPT)
				throw ExOps::EXCEPTION_INVALID_INPUT;
			return 1;
		}
	}
	// At this point PrevNumElems represents the number of elements in the arrays listed
	VectorIn = MexVector<T>(PrevNumElems);
	for (int i = 0; i < PrevNumElems; ++i){
		struct_inp_fun(StructFieldmxArrays, VectorIn[i]);
		// updating pointers to the next element
		for (auto Elem : StructFieldmxArrays){
			auto &CurrElem = StructFieldmxArrays[Elem.first];
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
	mxArray* tempmxArrayPtr = mxGetField(InputStruct, 0, FieldName);
	if (tempmxArrayPtr != NULL && !mxIsEmpty(tempmxArrayPtr)){
		size_t NumElems = mxGetNumberOfElements(tempmxArrayPtr);
		if (InputOps.REQUIRED_SIZE != -1 && InputOps.REQUIRED_SIZE != NumElems){
			if (!InputOps.QUIET)
				WriteOutput("The size of %s is required to be %d, it is currenty %d\n", FieldName, InputOps.REQUIRED_SIZE, NumElems);
			if (!InputOps.NO_EXCEPT)
				throw ExOps::EXCEPTION_INVALID_INPUT;
			return 1;
		}
		else{
			getInputfrommxArray<TypeSrc, TypeDest>(tempmxArrayPtr, VectorIn);
		}
	}
	else if (InputOps.IS_REQUIRED){
		if (!InputOps.QUIET)
			WriteOutput("The required field '%s' is either empty or non-existant.\n", FieldName);
		if (!InputOps.NO_EXCEPT)
			throw ExOps::EXCEPTION_INVALID_INPUT;
		return 1;
	}
	return 0;
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
	mxArray* tempmxArrayPtr = mxGetField(InputStruct, 0, FieldName);
	if (tempmxArrayPtr != NULL && !mxIsEmpty(tempmxArrayPtr)){
		size_t NumElems = mxGetNumberOfElements(tempmxArrayPtr);
		if (InputOps.REQUIRED_SIZE != -1 && InputOps.REQUIRED_SIZE != NumElems){
			if (!InputOps.QUIET)
				WriteOutput("The size of %s is required to be %d, it is currenty %d\n", FieldName, InputOps.REQUIRED_SIZE, NumElems);
			if (!InputOps.NO_EXCEPT)
				throw ExOps::EXCEPTION_INVALID_INPUT;
			return 1;
		}
		else{
			getInputfrommxArray<TypeSrc, TypeDest>(tempmxArrayPtr, VectorIn, casting_fun);
		}
	}
	else if (InputOps.IS_REQUIRED){
		if (!InputOps.QUIET)
			WriteOutput("The required field '%s' is either empty or non-existant.\n", FieldName);
		if (!InputOps.NO_EXCEPT)
			throw ExOps::EXCEPTION_INVALID_INPUT;
		return 1;
	}
	return 0;
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
	mxArray* tempmxArrayPtr = mxGetField(InputStruct, 0, FieldName);
	if (tempmxArrayPtr != NULL && !mxIsEmpty(tempmxArrayPtr)){
		size_t NumElems = mxGetNumberOfElements(tempmxArrayPtr);
		if (InputOps.REQUIRED_SIZE != -1 && InputOps.REQUIRED_SIZE != NumElems){
			if (!InputOps.QUIET)
				WriteOutput("The size of %s is required to be %d, it is currenty %d\n", FieldName, InputOps.REQUIRED_SIZE, NumElems);
			if (!InputOps.NO_EXCEPT)
				throw ExOps::EXCEPTION_INVALID_INPUT;
			return 1;
		}
		else{
			getInputfrommxArray<TypeSrc, TypeDest>(tempmxArrayPtr, VectorIn, casting_fun);
		}
	}
	else if (InputOps.IS_REQUIRED){
		if (!InputOps.QUIET)
			WriteOutput("The required field '%s' is either empty or non-existant.\n", FieldName);
		if (!InputOps.NO_EXCEPT)
			throw ExOps::EXCEPTION_INVALID_INPUT;
		return 1;
	}
	return 0;
}
#endif