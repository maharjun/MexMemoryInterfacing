#ifndef GENERIC_MEX_IO
#define GENERIC_MEX_IO

#include <matrix.h>
#include <mex.h>
#undef printf

#include <cstdarg>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cstdio>
#include <string.h>

#include "MexMem.hpp"
#include "LambdaToFunction.hpp"
#include "MexTypeTraits.hpp"

#ifdef _MSC_VER
#  define STRCMPI_FUNC _strcmpi
#elif defined __GNUC__
#  if (__GNUC__ > 5) || (__GNUC__ == 5)
#    define STRCMPI_FUNC strcasecmp
#  endif
#endif

typedef std::unordered_map<std::string, std::pair<void*, size_t> > StructArgTable;

//////////////////////////////////////////////////////////////////
///////////////////// BASIC HELPER FUNCTIONS /////////////////////
//////////////////////////////////////////////////////////////////

inline void vWriteOutput(const char *Format, std::va_list Args) {
	char buffertemp[256], bufferFinal[256];
	vsnprintf(buffertemp, 256, Format, Args);

	char* tempIter = buffertemp;
	char* FinalIter = bufferFinal;
	for (; *tempIter != 0; ++tempIter, ++FinalIter) {
		if (*tempIter == '%') {
			*FinalIter = '%';
			++FinalIter;
			*FinalIter = '%';
		}
		else {
			*FinalIter = *tempIter;
		}
	}
	*FinalIter = 0;
#ifdef MEX_LIB
	mexPrintf(bufferFinal);
	mexEvalString("drawnow();");
#elif defined MEX_EXE
	std::printf(bufferFinal);
	std::fflush(stdout);
#endif
}

inline void WriteOutput(const char *Format, ...) {
	std::va_list Args;
	va_start(Args, Format);
	vWriteOutput(Format, Args);
	va_end(Args);
}

template <typename ExType>
inline void WriteException(ExType Exception, const char *Format, ...) {
	std::va_list Args;
	va_start(Args, Format);
	vWriteOutput(Format, Args);
	va_end(Args);

	throw Exception;
}

inline void StringSplit(const char* InputString, const char* DelimString, std::vector<std::string> &SplitStringVect,
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

//////////////////////////////////////////////////////////////////
//////////////////////// OUTPUT FUNCTIONS ////////////////////////
//////////////////////////////////////////////////////////////////

template<typename TypeDest, typename T> inline mxArrayPtr assignmxArray(T &ScalarOut){

	mxClassID ClassID = GetMexType<TypeDest>::typeVal;
	mxArrayPtr ReturnPointer;

	if (std::is_arithmetic<T>::value){
		ReturnPointer = mxCreateNumericMatrix_730(1, 1, ClassID, mxREAL);
		*reinterpret_cast<TypeDest *>(mxGetData(ReturnPointer)) = (TypeDest)ScalarOut;
	}
	else{
		ReturnPointer = mxCreateNumericMatrix_730(0, 0, ClassID, mxREAL);
	}

	return ReturnPointer;
}

template<typename T, class Al, class B=typename std::enable_if<std::is_same<Al, mxAllocator>::value>::type>
inline mxArrayPtr assignmxArray(MexMatrix<T, Al> &MatrixOut){

	mxClassID ClassID = GetMexType<T>::typeVal;
	mxArrayPtr ReturnPointer = mxCreateNumericMatrix_730(0, 0, ClassID, mxREAL);
	MatrixOut.trim();

	if (MatrixOut.ncols() && MatrixOut.nrows()){
		mxSetM(ReturnPointer, MatrixOut.ncols());
		mxSetN(ReturnPointer, MatrixOut.nrows());
		mxSetData(ReturnPointer, MatrixOut.releaseArray());
	}

	return ReturnPointer;
}

template<typename T, class Al, class B=typename std::enable_if<std::is_same<Al, mxAllocator>::value>::type>
inline mxArrayPtr assignmxArray(MexVector<T, Al> &VectorOut){

	mxClassID ClassID = GetMexType<T>::typeVal;
	mxArrayPtr ReturnPointer = mxCreateNumericMatrix_730(0, 0, ClassID, mxREAL);
	VectorOut.trim();

	if (VectorOut.size()){
		mxSetM(ReturnPointer, VectorOut.size());
		mxSetN(ReturnPointer, 1);
		mxSetData(ReturnPointer, VectorOut.releaseArray());
	}
	return ReturnPointer;
}

template<typename T, class Al, class AlSub>
inline mxArrayPtr assignmxArray(MexVector<MexVector<T, Al>, AlSub> &VectorOut){

	mxClassID ClassID = GetMexType<T>::typeVal;
	mxArrayPtr ReturnPointer;
	VectorOut.trim();

	if (VectorOut.size()){
		ReturnPointer = mxCreateCellMatrix(VectorOut.size(), 1);

		size_t VectVectSize = VectorOut.size();
		for (int i = 0; i < VectVectSize; ++i){
			mxSetCell(ReturnPointer, i, assignmxArray(VectorOut[i]));
		}
	}
	else{
		ReturnPointer = mxCreateCellMatrix_730(0, 0);
	}
	return ReturnPointer;
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

//////////////////////////////////////////////////////////////////
//////////////// GENERAL PURPOSE HELPER FUNCTIONS ////////////////
//////////////////////////////////////////////////////////////////

inline MexMemInputOps getInputOps_v(int nOptions, va_list Options) {
	MexMemInputOps InputOps;
	for (int i = 0; i < nOptions; ++i) {
		char *CurrOption = va_arg(Options, char*);
		if (!STRCMPI_FUNC("IS_REQUIRED", CurrOption)) {
			InputOps.IS_REQUIRED = true;
		}
		else if (!STRCMPI_FUNC("QUIET", CurrOption)) {
			InputOps.QUIET = true;
		}
		else if (!STRCMPI_FUNC("NO_EXCEPT", CurrOption)) {
			InputOps.NO_EXCEPT = true;
		}
		else if (!STRCMPI_FUNC("REQUIRED_SIZE", CurrOption)) {
			InputOps.REQUIRED_SIZE = va_arg(Options, int);
		}
	}
	return InputOps;
}

inline MexMemInputOps getInputOps(int nOptions = 0, ...) {
	
	MexMemInputOps ReturnOps;
	va_list OptionArray;
	va_start(OptionArray, nOptions);
	ReturnOps = getInputOps_v(nOptions, OptionArray);
	va_end(OptionArray);

	return ReturnOps;
}

template <typename FieldCppType = void>
static const mxArray* getValidStructField(const mxArray* InputStruct, const char * FieldName, const MexMemInputOps & InputOps = MexMemInputOps()){
	
	// Processing Struct Name Heirarchy
	std::vector<std::string> NameHeirarchyVect;
	const mxArray* InputStructField = InputStruct;
	StringSplit(FieldName, ".", NameHeirarchyVect);
	
	// Validating wether InputStruct is not nullptr
	if (InputStruct == nullptr)
		return nullptr;

	int NameHeirarchyDepth = NameHeirarchyVect.size();
	for (int i = 0; i < NameHeirarchyDepth - 1; ++i){
		InputStructField = mxGetField(InputStructField, 0, NameHeirarchyVect[i].data());
		if (InputStructField == nullptr || mxIsEmpty(InputStructField) || mxGetClassID(InputStructField) != mxSTRUCT_CLASS){
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
	
	// Extracting Final Vector
	InputStructField = mxGetField(InputStructField, 0, NameHeirarchyVect.back().data());

	// Validate Type of Field
	if (!FieldInfo<FieldCppType>::CheckType(InputStructField)) {
		if (!InputOps.QUIET)
			WriteOutput("The Field '%s' does not match the type required.\n", FieldName);
		if (!InputOps.NO_EXCEPT)
			throw ExOps::EXCEPTION_INVALID_INPUT;
	}

	// Calculate Number of elements
	size_t NumElems = FieldInfo<FieldCppType>::getSize(InputStructField);

	// If Field exists with non-empty data
	if (InputStructField != nullptr) {
		if (InputOps.REQUIRED_SIZE != -1 && NumElems > 0 && InputOps.REQUIRED_SIZE != NumElems) {
			if (!InputOps.QUIET)
				WriteOutput("The size of %s is required to be %d, it is currenty %d\n", FieldName, InputOps.REQUIRED_SIZE, NumElems);
			if (!InputOps.NO_EXCEPT)
				throw ExOps::EXCEPTION_INVALID_INPUT;
			return nullptr;
		}
		else {
			return InputStructField;
		}
	}
	// If no data was found
	else if (InputOps.IS_REQUIRED) {
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
	typename MexVector<TypeRHS>::iterator const RHSVectorBeg, 
	typename MexVector<TypeRHS>::iterator const RHSVectorEnd,
	typename MexVector<TypeLHS>::iterator const LHSVectorBeg,
	std::function<void(TypeLHS &, TypeRHS &)> transform_func){

	auto RHSIter = RHSVectorBeg;
	auto LHSIter = LHSVectorBeg;
	for (; RHSIter != RHSVectorEnd; ++RHSIter, ++LHSIter){
		transform_func(*LHSIter , *RHSIter);
	}
	return LHSIter;
}

template<typename TypeRHS, typename TypeLHS>
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

//////////////////////////////////////////////////////////////////
////////////////////////// SCALAR INPUT //////////////////////////
//////////////////////////////////////////////////////////////////

// -------- From mxArray -------- //

template <typename TypeSrc, typename TypeDest> 
inline void getInputfrommxArray(const mxArray* InputArray, TypeDest &ScalarIn){
	if (InputArray != nullptr && !mxIsEmpty(InputArray))
		ScalarIn = (TypeDest)(*reinterpret_cast<TypeSrc *>(mxGetData(InputArray)));
}

template <typename TypeSrc, typename TypeDest>
inline void getInputfrommxArray(const mxArray* InputArray, TypeDest &ScalarIn,
	TypeDest(*casting_func)(TypeSrc &SrcElem)) {
	if (InputArray != nullptr && !mxIsEmpty(InputArray))
		ScalarIn = casting_func(*reinterpret_cast<TypeSrc *>(mxGetData(InputArray)));
}

template <typename TypeSrc, typename TypeDest>
inline void getInputfrommxArray(const mxArray* InputArray, TypeDest &ScalarIn,
	std::function<TypeDest(TypeSrc &)> &casting_func){
	if (InputArray != nullptr && !mxIsEmpty(InputArray))
		ScalarIn = casting_func(*reinterpret_cast<TypeSrc *>(mxGetData(InputArray)));
}

// -------- From Structure Field -------- //

template <typename TypeSrc, typename TypeDest>
inline int getInputfromStruct(const mxArray* InputStruct, const char* FieldName, TypeDest &ScalarIn, 
	MexMemInputOps InputOps = MexMemInputOps()) {

	InputOps.REQUIRED_SIZE = -1;
	const mxArray* StructFieldPtr = getValidStructField<TypeSrc>(InputStruct, FieldName, InputOps);

	if (StructFieldPtr != nullptr) {
		getInputfrommxArray<TypeSrc>(StructFieldPtr, ScalarIn);
		return 0;
	}
	else {
		return 1;
	}
}

template <typename TypeSrc, typename TypeDest>
inline int getInputfromStruct(const mxArray* InputStruct, const char* FieldName, TypeDest &ScalarIn,
	TypeDest(*casting_func)(TypeSrc &SrcElem),
	MexMemInputOps InputOps = MexMemInputOps()) {

	InputOps.REQUIRED_SIZE = -1;
	const mxArray* StructFieldPtr = getValidStructField<TypeSrc>(InputStruct, FieldName, InputOps);

	if (StructFieldPtr != nullptr) {
		getInputfrommxArray<TypeSrc>(StructFieldPtr, ScalarIn, casting_func);
		return 0;
	}
	else {
		return 1;
	}
}

template <typename TypeSrc, typename TypeDest>
inline int getInputfromStruct(const mxArray* InputStruct, const char* FieldName, TypeDest &ScalarIn,
	std::function<TypeDest(TypeSrc &)> &casting_func,
	MexMemInputOps InputOps = MexMemInputOps()) {

	InputOps.REQUIRED_SIZE = -1;
	const mxArray* StructFieldPtr = getValidStructField<TypeSrc>(InputStruct, FieldName, InputOps);

	if (StructFieldPtr != nullptr) {
		getInputfrommxArray<TypeSrc>(StructFieldPtr, ScalarIn, casting_func);
		return 0;
	}
	else {
		return 1;
	}
}

//////////////////////////////////////////////////////////////////
////////////////////////// VECTOR INPUT //////////////////////////
//////////////////////////////////////////////////////////////////

// -------- From mxArray -------- //

template <typename TypeSrc, typename TypeDest, class AlDest>
inline void getInputfrommxArray(
	const mxArray* InputArray,
	MexVector<TypeDest, AlDest> &VectorIn) {

	if (InputArray != nullptr && !mxIsEmpty(InputArray)) {
		size_t NumElems = mxGetNumberOfElements(InputArray);
		TypeSrc* tempArrayPtr = reinterpret_cast<TypeSrc*>(mxGetData(InputArray));
		VectorIn.resize(NumElems); // This will not erase old data
		for (int i = 0; i < NumElems; ++i) {
			VectorIn[i] = (TypeDest)tempArrayPtr[i];
		}
	}
}

template <typename TypeSrc, typename TypeDest, class AlDest>
inline void getInputfrommxArray(
	const mxArray* InputArray,
	MexVector<TypeDest, AlDest> &VectorIn,
	void(*casting_func)(TypeSrc &SrcElem, TypeDest &DestElem)) {

	if (InputArray != nullptr && !mxIsEmpty(InputArray)) {
		size_t NumElems = mxGetNumberOfElements(InputArray);
		TypeSrc* tempArrayPtr = reinterpret_cast<TypeSrc*>(mxGetData(InputArray));
		VectorIn.resize(NumElems);
		for (int i = 0; i < NumElems; ++i) {
			casting_func(tempArrayPtr[i], VectorIn[i]);
		}
	}
}

template <typename TypeSrc, typename TypeDest, class AlDest>
inline void getInputfrommxArray(
	const mxArray* InputArray,
	MexVector<TypeDest, AlDest> &VectorIn,
	std::function<void(TypeSrc &, TypeDest &)> &casting_func) {

	if (InputArray != nullptr && !mxIsEmpty(InputArray)) {
		size_t NumElems = mxGetNumberOfElements(InputArray);
		TypeSrc* tempArrayPtr = reinterpret_cast<TypeSrc*>(mxGetData(InputArray));
		VectorIn.resize(NumElems);
		for (int i = 0; i < NumElems; ++i) {
			casting_func(tempArrayPtr[i], VectorIn[i]);
		}
	}
}

// -------- From Structure Field -------- //

template <typename TypeSrc, typename TypeDest, class AlDest>
inline int getInputfromStruct(
	const mxArray* InputStruct, const char* FieldName,
	MexVector<TypeDest, AlDest> &VectorIn,
	MexMemInputOps InputOps = MexMemInputOps()) {

	// Processing Data
	const mxArray* StructFieldPtr = getValidStructField<MexVector<TypeSrc> >(InputStruct, FieldName, InputOps);
	if (StructFieldPtr != nullptr) {
		getInputfrommxArray<TypeSrc, TypeDest>(StructFieldPtr, VectorIn);
		return 0;
	}
	else {
		return 1;
	}
}

template <typename TypeSrc, typename TypeDest, class AlDest>
inline int getInputfromStruct(
	const mxArray* InputStruct, const char* FieldName,
	MexVector<TypeDest, AlDest> &VectorIn,
	void(*casting_func)(TypeSrc &SrcElem, TypeDest &DestElem),
	MexMemInputOps InputOps = MexMemInputOps()) {

	// Processing Data
	const mxArray* StructFieldPtr = getValidStructField<MexVector<TypeSrc> >(InputStruct, FieldName, InputOps);
	if (StructFieldPtr != nullptr) {
		getInputfrommxArray<TypeSrc, TypeDest>(StructFieldPtr, VectorIn, casting_func);
		return 0;
	}
	else {
		return 1;
	}
}

template <typename TypeSrc, typename TypeDest, class AlDest>
inline int getInputfromStruct(
	const mxArray* InputStruct, const char* FieldName,
	MexVector<TypeDest, AlDest> &VectorIn,
	std::function<void(TypeSrc &, TypeDest &)> &casting_func,
	MexMemInputOps InputOps = MexMemInputOps()) {

	// Processing Data
	const mxArray* StructFieldPtr = getValidStructField<MexVector<TypeSrc> >(InputStruct, FieldName, InputOps);
	if (StructFieldPtr != nullptr) {
		getInputfrommxArray<TypeSrc, TypeDest>(StructFieldPtr, VectorIn, casting_func);
		return 0;
	}
	else {
		return 1;
	}
}

//////////////////////////////////////////////////////////////////
////////////////////////// MATRIX INPUT //////////////////////////
//////////////////////////////////////////////////////////////////

// -------- From mxArray -------- //

template <typename TypeSrc, typename TypeDest, class AlDest>
inline void getInputfrommxArray(
	const mxArray* InputArray,
	MexMatrix<TypeDest, AlDest> &MatrixIn) {

	if (InputArray != nullptr && !mxIsEmpty(InputArray)) {
		size_t NDim0 = FieldInfo<decltype(MatrixIn)>::getSize(InputArray, 0);
		size_t NDim1 = FieldInfo<decltype(MatrixIn)>::getSize(InputArray, 1);

		TypeSrc* tempArrayPtr = reinterpret_cast<TypeSrc*>(mxGetData(InputArray));
		MatrixIn.resize(NDim1, NDim0); // This will not erase old data
		for (int i=0; i<NDim1; ++i) {
			for (int j=0; j<NDim0; ++j) {
				MatrixIn(i,j) = (TypeDest)tempArrayPtr[NDim0*i + j];
			}
		}
	}
}

template <typename TypeSrc, typename TypeDest, class AlDest>
inline void getInputfrommxArray(
	const mxArray* InputArray,
	MexMatrix<TypeDest, AlDest> &MatrixIn,
	void(*casting_func)(TypeSrc &SrcElem, TypeDest &DestElem)) {

	if (InputArray != nullptr && !mxIsEmpty(InputArray)) {
		size_t NDim0 = FieldInfo<decltype(MatrixIn)>::getSize(InputArray, 0);
		size_t NDim1 = FieldInfo<decltype(MatrixIn)>::getSize(InputArray, 1);

		TypeSrc* tempArrayPtr = reinterpret_cast<TypeSrc*>(mxGetData(InputArray));
		MatrixIn.resize(NDim1, NDim0); // This will not erase old data
		for (int i=0; i<NDim1; ++i) {
			for (int j=0; j<NDim0; ++j) {
				casting_func(tempArrayPtr[NDim0*i + j], MatrixIn(i,j));
			}
		}
	}
}

template <typename TypeSrc, typename TypeDest, class AlDest>
inline void getInputfrommxArray(
	const mxArray* InputArray,
	MexMatrix<TypeDest, AlDest> &MatrixIn,
	std::function<void(TypeSrc &, TypeDest &)> &casting_func) {

	if (InputArray != nullptr && !mxIsEmpty(InputArray)) {
		size_t NDim0 = FieldInfo<decltype(MatrixIn)>::getSize(InputArray, 0);
		size_t NDim1 = FieldInfo<decltype(MatrixIn)>::getSize(InputArray, 1);

		TypeSrc* tempArrayPtr = reinterpret_cast<TypeSrc*>(mxGetData(InputArray));
		MatrixIn.resize(NDim1, NDim0); // This will not erase old data
		for (int i=0; i<NDim1; ++i) {
			for (int j=0; j<NDim0; ++j) {
				casting_func(tempArrayPtr[NDim0*i + j], MatrixIn(i,j));
			}
		}
	}
}

// -------- From Structure Field -------- //

template <typename TypeSrc, typename TypeDest, class AlDest>
inline int getInputfromStruct(
	const mxArray* InputStruct, const char* FieldName,
	MexMatrix<TypeDest, AlDest> &MatrixIn,
	MexMemInputOps InputOps = MexMemInputOps()) {

	// Processing Data
	const mxArray* StructFieldPtr = getValidStructField<MexMatrix<TypeSrc> >(InputStruct, FieldName, InputOps);
	if (StructFieldPtr != nullptr) {
		getInputfrommxArray<TypeSrc, TypeDest>(StructFieldPtr, MatrixIn);
		return 0;
	}
	else {
		return 1;
	}
}

template <typename TypeSrc, typename TypeDest, class AlDest>
inline int getInputfromStruct(
	const mxArray* InputStruct, const char* FieldName,
	MexMatrix<TypeDest, AlDest> &MatrixIn,
	void(*casting_func)(TypeSrc &SrcElem, TypeDest &DestElem),
	MexMemInputOps InputOps = MexMemInputOps()) {

	// Processing Data
	const mxArray* StructFieldPtr = getValidStructField<MexMatrix<TypeSrc> >(InputStruct, FieldName, InputOps);
	if (StructFieldPtr != nullptr) {
		getInputfrommxArray<TypeSrc, TypeDest>(StructFieldPtr, MatrixIn, casting_func);
		return 0;
	}
	else {
		return 1;
	}
}

template <typename TypeSrc, typename TypeDest, class AlDest>
inline int getInputfromStruct(
	const mxArray* InputStruct, const char* FieldName,
	MexMatrix<TypeDest, AlDest> &MatrixIn,
	std::function<void(TypeSrc &, TypeDest &)> &casting_func,
	MexMemInputOps InputOps = MexMemInputOps()) {

	// Processing Data
	const mxArray* StructFieldPtr = getValidStructField<MexMatrix<TypeSrc> >(InputStruct, FieldName, InputOps);
	if (StructFieldPtr != nullptr) {
		getInputfrommxArray<TypeSrc, TypeDest>(StructFieldPtr, MatrixIn, casting_func);
		return 0;
	}
	else {
		return 1;
	}
}

//////////////////////////////////////////////////////////////////
///////////////////////// VECTVECT INPUT /////////////////////////
//////////////////////////////////////////////////////////////////

// -------- From mxArray -------- //

template <typename T, class AlSub, class Al>
inline void getInputfrommxArray(const mxArray* InputArray, MexVector<MexVector<T, AlSub>, Al> &VectorIn){
	if (InputArray != nullptr && !mxIsEmpty(InputArray) && mxGetClassID(InputArray) == mxCELL_CLASS){
		size_t NumElems = mxGetNumberOfElements(InputArray);
		mxArrayPtr* tempArrayPtr = reinterpret_cast<mxArrayPtr*>(mxGetData(InputArray));
		VectorIn = MexVector<MexVector<T> >(NumElems, MexVector<T>(0));
		for (int i = 0; i < NumElems; ++i){
			getInputfrommxArray<T>(tempArrayPtr[i], VectorIn[i]);
		}
	}
}

// -------- From Structure Field -------- //

template <typename T, class AlSub, class Al> 
inline int getInputfromStruct(
	const mxArray* InputStruct, const char* FieldName, 
	MexVector<MexVector<T, AlSub>, Al> &VectorIn, 
	MexMemInputOps InputOps = MexMemInputOps()) {

	// Processing Data
	const mxArray * StructFieldPtr = getValidStructField<MexVector<MexVector<T> > >(InputStruct, FieldName, InputOps);
	if (StructFieldPtr != nullptr) {
		getInputfrommxArray<T>(StructFieldPtr, VectorIn);
		return 0;
	}
	else {
		return 1;
	}
}

//////////////////////////////////////////////////////////////////
////////////////////////// STRUCT INPUT //////////////////////////
//////////////////////////////////////////////////////////////////

// This function is slightly less secure as strict type compliance is not ensured
template <typename T, class Al>
inline int getInputfromStruct(
	const mxArray* InputStruct, const char* FieldName, 
	MexVector<T, Al> &VectorIn, 
	void(*struct_inp_fun)(StructArgTable &ArgumentVects, T &DestElem),
	MexMemInputOps InputOps = MexMemInputOps()){

	// Processing Data
	// converting Field Name into array of field names by splitting at '-'
	std::vector<std::string> FieldNamesVect(0);
	StructArgTable StructFieldmxArrays;

	// Split the string into its components by delimeters ' -/,'
	StringSplit(FieldName, " -/,", FieldNamesVect);

	// For each valid field name stored in vector, add entry in map
	size_t PrevNumElems = 0; // used to enforce condition that all are of equal size

	for (int i = 0; i < FieldNamesVect.size(); ++i){

		const mxArray * StructFieldPtr;

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
				std::pair<std::string, std::pair<void*, size_t> >(
					FieldNamesVect[i], 
					std::pair<void*, size_t>(mxGetData(StructFieldPtr), ElemSize)
				));
			PrevNumElems = NumElems;
		}
		else{
			// Store the null pointer and 0 size for current field
			StructFieldmxArrays.insert(
				std::pair<std::string, std::pair<void*, size_t> >(
					FieldNamesVect[i],
					std::pair<void*, size_t>(nullptr, 0)
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
// This function is slightly less secure as strict type compliance is not ensured
template <typename T, class Al>
inline int getInputfromStruct(
	const mxArray* InputStruct, const char* FieldName,
	MexVector<T, Al> &VectorIn,
	const std::function<void(StructArgTable &ArgumentVects, T &DestElem)> &struct_inp_fun,
	MexMemInputOps InputOps = MexMemInputOps()){

	// Processing Data
	// converting Field Name into array of field names by splitting at '-'
	std::vector<std::string> FieldNamesVect(0);
	StructArgTable StructFieldmxArrays;

	// Split the string into its components by delimeters ' -/,'
	StringSplit(FieldName, " -/,", FieldNamesVect);

	// For each valid field name stored in vector, add entry in map
	size_t PrevNumElems = 0; // used to enforce condition that all are of equal size

	for (int i = 0; i < FieldNamesVect.size(); ++i){

		const mxArray * StructFieldPtr;

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
				std::pair<std::string, std::pair<void*, size_t> >(
				FieldNamesVect[i],
				std::pair<void*, size_t>(mxGetData(StructFieldPtr), ElemSize)
				));
			PrevNumElems = NumElems;
		}
		else{
			// Store the null pointer and 0 size for current field
			StructFieldmxArrays.insert(
				std::pair<std::string, std::pair<void*, size_t> >(
				FieldNamesVect[i],
				std::pair<void*, size_t>(nullptr, 0)
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

#endif