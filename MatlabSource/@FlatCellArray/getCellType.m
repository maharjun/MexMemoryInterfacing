function [ TypeString ] = getCellType( CellArray )
%GETCELLTYPE Gets the type (if consistant) of the vectors stored in the
%input Cell Array
%   
%   [ TypeString ] = getCellType( CellArray )
%   
%   Input Arguments:
%   
%     A cell array (CellArray)
%   
%   Output Arguments:
%   
%     a string indicating a type -
%     
%     'undecided'    - returned when all the cell array elements are empty.
%                      Note that this means that every cell array in the
%                      list is empty. This value is not returned if there
%                      are any Final level vectors (even empty). In such
%                      case,the type of the empty vector is considered.
%     
%     'notcellarray' - returned when the input array is not a cell array
%     
%     'error'        - The all the non-undecided types of elements at any 
%                      particular level dont match. This is also returned
%                      if the calculated common type is 'struct'
%     
%     '<classid>'    - the name of the class of objects stored. (e.g.
%                      'double', 'int32', 'single', 'uint32', 'char'
%     

	if ~iscell(CellArray)
		TypeString = 'notcellarray';
		return;
	end
	
	% Handling the case where cell array is empty.
	if isempty(CellArray)
		TypeString = 'undecided';
		return;
	end
	
	% If not empty, find all non-cell elements
	NonCellInds = ~cellfun(@iscell, CellArray);
	
	% Validate number of non-cell elements
	if nnz(NonCellInds) ~= length(CellArray) && nnz(NonCellInds) ~= 0
		TypeString = 'error';
		return;
	end
	
	% Find the type(s) of the non-cell elements -
	% The result (by above check) should be either empty (All elements are
	% cells), or a single non-cell type (All elements belong to the
	% particular non-cell type)
	CurrentUniqueTypes = unique(cellfun(@class, CellArray(NonCellInds), 'UniformOutput', false));
	if length(CurrentUniqueTypes) > 1
		TypeString = 'error';
	elseif isempty(CurrentUniqueTypes)
		% In the event that CellArray contains cell arrays
		% Getting the unique types of the sub cell arrays
		CurrentUniqueTypes = unique(cellfun(@FlatCellArray.getCellType, CellArray, 'UniformOutput', false));

		% Removing the occurrences of 'undecided'
		CurrentUniqueTypes = CurrentUniqueTypes(cellfun(@(x) ~strcmp(x, 'undecided'), CurrentUniqueTypes));

		% Validating the types
		if length(CurrentUniqueTypes) > 1
			TypeString = 'error';
		elseif isempty(CurrentUniqueTypes)
			TypeString = 'undecided';
		else
			TypeString = CurrentUniqueTypes{1};
		end
	else
		if ~strcmp(CurrentUniqueTypes{1}, 'struct')
			TypeString = CurrentUniqueTypes{1};
		else
			TypeString = 'error';
		end
	end
end