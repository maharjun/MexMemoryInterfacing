function LevelSizeVect = getCellLevelSizes(CellArray)
% getCellLevelSizes - Returns vector of total size at each level of cell array
% 
%     LevelSizeVect = getCellLevelSizes(CellArray)
% 
%   This function returns the vector of total size at each 
%   level of the cell array CellArray. as in -
%     
%     LevelSizeVect(1)       = length(CellArray);
%     LevelSizeVect(2)       = sum(length(elem) for each elem in CellArray);
%     :
%     :
%     LevelSizeVect(Depth+1) = Length of Total Data Vector
%   
%   This function relies on the fact that the given CellArray is valid
%   (i.e. the type returned by getCelltype should not be 'error'). If
%   otherwise, the behaviour is undefined. 
%   
%   Note however that 'error' from getCelltype may actually be because
%   CellArray is a vector (i.e. not a cell array); however, that is a
%   defined case for this function which will simply return the length
%   of CellArray in that case.

	if ~iscell(CellArray)
		% This corresponds to the case where we are finding the
		% LevelSizeVect for one of the final level vectors. In such
		% case, CellArray is not really a Cell Array but rather a
		% vector (or a struct vector)
		LevelSizeVect = length(CellArray);
		return;
	end

	NCurrLevelElems = length(CellArray);
	CellArrayDepth = FlatCellArray.getCellDepth(CellArray);
	LevelSizeElems = zeros(CellArrayDepth, 1);
	for i=1:NCurrLevelElems
		LevelSizeElems = LevelSizeElems + FlatCellArray.getCellLevelSizes(CellArray{i});
	end
	
	LevelSizeVect = [NCurrLevelElems; LevelSizeElems];

end

