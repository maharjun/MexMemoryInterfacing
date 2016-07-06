MexMemoryInterfacing
======================================

##  Introduction

MATLAB is among the most popular softwares in existence for numerical computation. It is widely used in all manner of scientific investigations including electrical, mechanical, chemical, [you name it] engineering.

As it turns out, most newbies to MATLAB find it challenging to adapt to coding in MATLAB when they realize that the code that they write in C, C++, Java etc., as written in C, C++, Java, runs painfully slowly under MATLAB. The reason for this is MATLAB's nature as an interpreted language where there is a significant runtime overhead to running every line of code (for e.g., the data type compatibility must be checked, indexing validity must be checked etc.). This puts the requirement on the programmer that a large amount of work is done in each statement i.e. the requirement that the code be "vectorized".

HOWEVER, as it so happens, some algorithms simply cannot be wrtten in a vectorized manner. A simple example would be to find the cumulative sum of a given vector. Fortunately, matlab includes a function that efficiently executes the above computation. However, there are several other, less fortunate cases where the best one seems to be able to do, is write a for loop, and lament as MATLAB takes an eternity to execute the code.

Having recognized the above problem, MATLAB provides an interface to use C/C++ to program, called MATLAB EXtensions or MEX for short. This interface is an elegant C interface that enables us to access the underlying data in existing MATLAB vectors/matrices, and perform the non-vectorizable computation in the FAST C language, following which we can return data back to MATLAB in a MATLAB vector/matrix.

However, as C++ programmers, we are used to the neat functionality offered by the C++ Standard Library (e.g. std::vector), static time polymorphism, and especially RAII. These features are entirely missing if we only restrict ourselves to the C interface provided by MEX.

I Have found out that one CANNOT use the C++ standard library to manipulate memory that is returned to MATLAB due to various reasons (e.g. Alignment Requirements and compulsory deallocation at the end of scope). 

Thus, I have developed a set of headers in my repository MexMemoryInterfacing which implements two classes MexVector and MexMatrix that allow me to perform a zero-cost wrapping over MATLAB Data. The Additional features are listed below.

##  Relevant Code

The relevant code can be found in the repository titled MexMemoryInterfacing at the following link. Only the subfolders `Headers` and `MatlabSource` contain relevant code.

https://github.com/maharjun/MexMemoryInterfacing

The code in the following repositories uses MexMemoryInterfacing

https://github.com/maharjun/TimeDelNetSim

https://github.com/maharjun/PolychronousGroupFind

https://github.com/maharjun/Grid2D

https://github.com/maharjun/GetAttractionBasin

##  Current Features

The features that have currently been implemented are as below.

1.  `MexVector<DataType>` can wrap any MATLAB Vector of any data type.
2.  `MexMatrix<DataType>` can wrap any MATLAB Matrix of any data type.
3.  `MexVector` attempts to mirror the interface offered by std::vector
4.  `MexVector` has interfaces that allow it interact with standard library iterators
5.  `MexVector` is capable of wrapping cell arrays as `MexVector<MexVector<...>>`
6.  I have programmed templated input and output functions that perform type inference and type checking. I have emphasized static typing where I felt the need.
7.  I have functions that perform Ctrl-C (Interrupt Signal) Handling
8.  _Include Other Features Later_

##  Current Issues

1.  Heavily uses C++11 features and is only compilable under gcc 4.9.2 or greater. (All of my code has been written assuming gcc 5 (i.e. complete C++11 support)). It is also compilable under Visual Studio 13 or 15.
2.  Lack of comprehensive **testing** and **documentation**.
3.  Requires knowledge of template programming in order to implement new data structures. (Need to simplify process).
4.  Input Output functions may need some polishing / refactoring.
5.  Implementation of **Ownership** features is very limited.
6.  Havent used a namespace
7.  Need to investigate the extent to which this can be linked to C++ Numerical computation software (specifically eigen).

##  Why Contribute?

1.  Will gain in-depth understanding of C++ and template programming upon reading the source code.
2.  MATLAB is widely used and there is no question regarding the utility of this project.
3.  Will likely be useful to yourself if you are a regular user of MATLAB and are frustrated by its slowness for your particular application.

##  Required Skills.

Well, All of the skills can be learned on the job (willing to mentor). But, in order to contribute, you will need to be familiar with the following

1.  MATLAB syntax (at least the basic details)
2.  MEX Interface. It has an intuitive and clear design. Should not take too long.
3.  This is the hardest. C++ Template Metaprogramming (necessary to understand the code, conversely, can be learnt by understanding the code).
