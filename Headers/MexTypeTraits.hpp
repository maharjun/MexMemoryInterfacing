#ifndef MEX_TYPE_TRAITS_HPP
#define MEX_TYPE_TRAITS_HPP

#include <matrix.h>
#include <stdint.h>
#include <type_traits>

#include "MexMem.hpp"

template <typename T>
struct GetMexType {
	static constexpr const uint32_t typeVal = mxUNKNOWN_CLASS;
};

template <> struct GetMexType<                    char16_t > { static constexpr const mxClassID typeVal = ::mxCHAR_CLASS   ; };
template <> struct GetMexType<   signed           char     > { static constexpr const mxClassID typeVal = ::mxINT8_CLASS   ; };
template <> struct GetMexType< unsigned           char     > { static constexpr const mxClassID typeVal = ::mxUINT8_CLASS  ; };
template <> struct GetMexType<   signed short     int      > { static constexpr const mxClassID typeVal = ::mxINT16_CLASS  ; };
template <> struct GetMexType< unsigned short     int      > { static constexpr const mxClassID typeVal = ::mxUINT16_CLASS ; };
template <> struct GetMexType<   signed           int      > { static constexpr const mxClassID typeVal = ::mxINT32_CLASS; };
template <> struct GetMexType< unsigned           int      > { static constexpr const mxClassID typeVal = ::mxUINT32_CLASS; };
template <> struct GetMexType<   signed long      int      > { static constexpr const mxClassID typeVal = ::mxINT32_CLASS  ; };
template <> struct GetMexType< unsigned long      int      > { static constexpr const mxClassID typeVal = ::mxUINT32_CLASS ; };
template <> struct GetMexType<   signed long long int      > { static constexpr const mxClassID typeVal = ::mxINT64_CLASS  ; };
template <> struct GetMexType< unsigned long long int      > { static constexpr const mxClassID typeVal = ::mxUINT64_CLASS ; };
template <> struct GetMexType<                    float    > { static constexpr const mxClassID typeVal = ::mxSINGLE_CLASS ; };
template <> struct GetMexType<                    double   > { static constexpr const mxClassID typeVal = ::mxDOUBLE_CLASS ; };

template <typename T, class Al>              struct GetMexType<MexVector<T, Al> >                   { static constexpr const uint32_t typeVal = GetMexType<T>::typeVal; };
template <typename T, class AlSub, class Al> struct GetMexType<MexVector<MexVector<T, AlSub>, Al> > { static constexpr const uint32_t typeVal = mxCELL_CLASS; };

// Type Traits extraction for Vectors
template <typename T, typename B = void> 
	struct isMexVector 
		{ static constexpr const bool value = false; };
template <typename T, class Al> 
	struct isMexVector<MexVector<T, Al>, typename std::enable_if<std::is_arithmetic<T>::value >::type > 
		{ static constexpr const bool value = true; typedef T type; };

// Type Traits extraction for Vector of Vectors
template <typename T, class B = void>
	struct isMexVectVector 
		{ static constexpr const bool value = false; };
template <typename T, class Al>
	struct isMexVectVector<MexVector<T, Al>, typename std::enable_if<isMexVector<T, Al>::value>::type > 
	{
		static constexpr const bool value = true;
		typedef typename isMexVector<T, Al>::type type;
		typedef T elemType;
	};
template <typename T, class Al>
	struct isMexVectVector<MexVector<T, Al>, typename std::enable_if<isMexVectVector<T, Al>::value>::type >
	{
		static constexpr const bool value = true;
		typedef typename isMexVectVector<T, Al>::type type;
		typedef T elemType;
	};

inline bool isMexVectorType(mxClassID ClassIDin) {
	switch (ClassIDin) {
		case mxINT8_CLASS   :
		case mxUINT8_CLASS  :
		case mxINT16_CLASS  :
		case mxUINT16_CLASS :
		case mxINT32_CLASS  :
		case mxUINT32_CLASS :
		case mxINT64_CLASS  :
		case mxUINT64_CLASS :
		case mxSINGLE_CLASS :
		case mxDOUBLE_CLASS :
			return true;
			break;
		default:
			return false;
	}
}

// Default Type Checking
template<typename T = void, class B = void>
struct FieldInfo {
	static inline bool CheckType(mxArrayPtr InputmxArray) {
		return false;
	}
	static inline uint32_t getSize(mxArrayPtr InputmxArray) {
		return 0;
	}
};

// Non Type Checking
template<>
struct FieldInfo<void> {
	static inline bool CheckType(mxArrayPtr InputmxArray) {
		return true;
	}
	static inline uint32_t getSize(mxArrayPtr InputmxArray) {
		size_t NumElems = 0;

		// If array is non-empty, calculate size
		if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray)) {
			NumElems = mxGetNumberOfElements(InputmxArray);
		}
		return NumElems;
	}
};

// Type Checking for scalar types
template<typename T>
struct FieldInfo<T, typename std::enable_if<std::is_arithmetic<T>::value >::type> {
	static inline bool CheckType(mxArrayPtr InputmxArray) {
		return (InputmxArray == nullptr || mxIsEmpty(InputmxArray) || mxGetClassID(InputmxArray) == GetMexType<T>::typeVal);
	}
	static inline uint32_t getSize(mxArrayPtr InputmxArray) {
		size_t NumElems = 0;

		// If array is non-empty, calculate size
		if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray)) {
			NumElems = mxGetNumberOfElements(InputmxArray);
		}
		return NumElems;
	}

};

// Type Checking for Vector of Scalars
template<typename T>
struct FieldInfo<T, typename std::enable_if<isMexVector<T>::value>::type> {
	static inline bool CheckType(mxArrayPtr InputmxArray) {
		return (InputmxArray == nullptr || mxIsEmpty(InputmxArray) || mxGetClassID(InputmxArray) == GetMexType<isMexVector<T>::type>::typeVal);
	}
	static inline uint32_t getSize(mxArrayPtr InputmxArray) {
		size_t NumElems = 0;

		// If array is non-empty, calculate size
		if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray)) {
			NumElems = mxGetNumberOfElements(InputmxArray);
		}
		return NumElems;
	}
};

// Type Checking for Cell Array (Vector Tree / Vector of Vectors)
template<typename T>
struct FieldInfo<T, typename std::enable_if<isMexVectVector<T>::value>::type> {
	static inline bool CheckType(mxArrayPtr InputmxArray) {
		bool isValid = true;
		if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray) && mxIsCell(InputmxArray)) {
			mxArrayPtr * SubVectorArray = reinterpret_cast<mxArrayPtr *>(mxGetData(InputmxArray));
			size_t NSubElems = mxGetNumberOfElements(InputmxArray);
			// Validate each subvector
			for (int i = 0; i < NSubElems; ++i) {
				if (!VAT<typename isMexVectVector<T>::elemType>(SubVectorArray[i])) {
					isValid = false;
					break;
				}
			}
		}
		else if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray)) {
			isValid = false;
		}
		else {
			isValid = true;
		}
		return isValid;
	}
	static inline uint32_t getSize(mxArrayPtr InputmxArray) {
		size_t NumElems = 0;

		// If array is non-empty, calculate size
		if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray)) {
			NumElems = mxGetNumberOfElements(InputmxArray);
		}
		return NumElems;
	}
};

#endif