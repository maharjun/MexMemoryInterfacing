#ifndef GENERIC_MEX_IO
#define GENERIC_MEX_IO

#include <matrix.h>
#include <mex.h>
#include <cstdarg>
#include "MexMem.hpp"

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

#endif