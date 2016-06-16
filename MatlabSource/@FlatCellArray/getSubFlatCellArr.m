function FlatCellArrOut = getSubFlatCellArr(FlatCellArrIn, Level, Indices )
%GETSUBFLATCELLARR - returns sub-FlatCellArray as specified by Indices
%   
%   It returns a cell array that is a subset / subelement of FlatCellArrIn
%   That is of a depth = FlatCellArrIn - Level + 1 that contains the
%   elements in the Level'th level indexed by Indices (1-start).
%   
%   If the Level = FlatCellArrIn.Depth + 1, the corresponding vector is
%   returned instead of a FlatCellArray
	
	if Level == FlatCellArrIn.Depth + 1
		FlatCellArrOut = FlatCellArrIn.Data(Indices);
	else
		PartitionIndexOut = cell(FlatCellArrIn.Depth - Level + 1, 1);
		CurrLevelIndices = Indices;
		for i = 1:length(PartitionIndexOut)
			% Calculating current level entries by performing cumsum over
			% the sizes of the current level elements
			CurrLevelElemSizes = [0; FlatCellArrIn.PartitionIndex{i+Level-1}(CurrLevelIndices+1) - FlatCellArrIn.PartitionIndex{i+Level-1}(CurrLevelIndices)];
			CurrLevelEntries = uint32(cumsum(CurrLevelElemSizes));
			PartitionIndexOut{i} = CurrLevelEntries;
			
			% Getting the next level indices using the values stored in the
			% current level at the current level indices.
			CurrLevelIndices = arrayfun(@(x, y) (x:y)', ...
				FlatCellArrIn.PartitionIndex{i+Level-1}(CurrLevelIndices)+1, FlatCellArrIn.PartitionIndex{i+Level-1}(CurrLevelIndices+1), ...
				'UniformOutput', false);
			CurrLevelIndices = cell2mat(CurrLevelIndices);
		end
		% Filling out Data
		DataOut = FlatCellArrIn.Data(CurrLevelIndices);

		FlatCellArrOut = FlatCellArray([], PartitionIndexOut, DataOut);
	end
end

