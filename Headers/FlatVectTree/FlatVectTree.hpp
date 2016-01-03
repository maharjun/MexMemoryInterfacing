#ifndef FLAT_VECT_TREE_HPP
#define FLAT_VECT_TREE_HPP

#include <type_traits>
#include <stdint.h>

#include "VectTreeInfo.hpp"
#include "../MexMem.hpp"
#include "../GenericMexIO.hpp"
#include "../MexTypeTraits.hpp"

enum FV_ExCodes {
    FV_INVALID_APPEND = 0x01,
	FV_INVALID_FETCH  = 0x02
};

template<typename T, class FVT_Al = mxAllocator, class B = typename std::enable_if< std::is_arithmetic<T>::value >::type>
class FlatVectTree {

	MexVector<MexVector<uint32_t, FVT_Al>, FVT_Al> PartitionIndex;
	MexVector<T, FVT_Al> Data;

	template<class Al>
	inline void getVectTreeFromInds(MexVector<T, Al> &VectTreeOut, uint32_t Level, uint32_t LevelIndex);
	template<typename SubElemT, class AlSub, class Al>
	inline void getVectTreeFromInds(MexVector<MexVector<SubElemT, AlSub>, Al> &VectTreeOut, uint32_t Level, uint32_t LevelIndex);
	
	template<class Al>
	inline void appendFast(const MexVector<T, Al> &VectIn);
	template<typename SubElemT, class AlSub, class Al>
	inline void appendFast(const MexVector<MexVector<SubElemT, AlSub>, Al > &VectTreeIn);
	
	template<class Al>
	inline void appendFast(MexVector<T, Al> &&VectIn);
	template<typename SubElemT, class AlSub, class Al>
	inline void appendFast(MexVector<MexVector<SubElemT, AlSub>, Al > &&VectTreeIn);

	template<typename, class, class>
	friend class FlatVectTree;

public:
	

	// Constructors
	inline FlatVectTree() : PartitionIndex(), Data() {}
	inline FlatVectTree(int Depth) : PartitionIndex(Depth, MexVector<uint32_t>(1, uint32_t(0))), Data() {}

	// Property Reassignment Functions
	inline bool setDepth(uint32_t NewDepth);
	inline void clear();
	inline void empty();

	// Assignment Functions
	template<class AlSub, class Al, class AlData>
	inline void assign(const MexVector<MexVector<uint32_t, AlSub>, Al> &PartitionIndexIn, const MexVector<T, AlData> & DataIn, bool ActualCopy = true);
	template<class FVT_Al2>
	inline void assign(const FlatVectTree<T, FVT_Al2> &FlatVectTreeIn, bool ActualCopy = true);

	// Appending Functions
	template<typename SubElemT, class Al>
	inline void append(const MexVector<SubElemT, Al> &SubElemTree, int InsertDepth = -1);

	// Move-Appending functions
	template<typename SubElemT, class Al>
	inline void append(MexVector<SubElemT, Al> &&SubElemTree, int InsertDepth = -1);

	// Push-Back Functions
	template<typename SubElemT, class Al>
	inline void push_back(const MexVector<SubElemT, Al> &MexVectIn, int InsertDepth = -1);

	// Move-Push-Back Functions
	template<typename SubElemT, class Al>
	inline void push_back(MexVector<SubElemT, Al> &&MexVectIn, int InsertDepth = -1);

	// Get Vector Tree
	template<typename SubElemT, class Al, class AlInds>
	inline void getVectTree(MexVector<SubElemT, Al> &VectTreeOut, const MexVector<uint32_t, AlInds> &Indices = MexVector<uint32_t>(0));
	template<typename SubElemT, class Al>
	inline void getVectTree(MexVector<SubElemT, Al> &VectTreeOut, uint32_t NIndices = 0, ...);

	// Release-Memory Functions
	inline void releaseMem(MexVector<MexVector<uint32_t, FVT_Al>, FVT_Al> &ReleasedPartInds, MexVector<T, FVT_Al> &ReleasedData);

    // Property Access Functions
    inline uint32_t depth()                        {
        return PartitionIndex.size();
    }
	inline uint32_t LevelSize(uint32_t LevelIndex) {
		return PartitionIndex[LevelIndex].size() - 1;
	}
	inline bool     isempty()                      {
		return (this->depth() == 0 || PartitionIndex[0].size() == 1);
	}
	inline bool     istrulyempty()                 {
		return (this->depth() == 0);
	}
	// Static Functions
	template<class AlSub, class Al, class AlData>
	static inline bool isValidFVT(const MexVector<MexVector<uint32_t, AlSub>, Al>& PartitionInds, const MexVector<T, AlData>& Data);
};

template <typename T, class Enable = void>
struct isFlatVectTree { static constexpr bool value = false; };
template <typename T, class Al>
struct isFlatVectTree<FlatVectTree<T, Al>, typename std::enable_if<std::is_arithmetic<T>::value>::type> { 
	static constexpr bool value = true; 
	typedef T type;
};

template <typename T>
struct FieldInfo<T, typename std::enable_if<isFlatVectTree<T>::value>::type> {
	static inline bool CheckType(const mxArray* InputmxArray);
	static inline uint32_t getSize(const mxArray* InputmxArray);
	static inline uint32_t getDepth(const mxArray* InputmxArray);
	static inline void moveIntoVectors(const mxArray* InputmxArray, 
		MexVector<MexVector<uint32_t> >   &PartitionIndexIn, 
		MexVector<typename isFlatVectTree<T>::type> &Data);
};

template <typename T> inline mxArrayPtr assignmxArray(FlatVectTree<T> &FlatVectTreeOut);
template <typename T, class Al> static void getInputfrommxArray(const mxArray *InputArray, FlatVectTree<T, Al> &FlatVectTreeIn);
template <typename T, class Al> static int getInputfromStruct(const mxArray *InputStruct, const char* FieldName, FlatVectTree<NonDeduc(T), Al> &FlatVectTreeIn, uint32_t RequiredDepth, MexMemInputOps InputOps = MexMemInputOps());

#include "FlatVectTree.inl"
#include "FlatVectTreeIO.inl"
#endif
