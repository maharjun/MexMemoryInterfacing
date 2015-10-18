function [ CellArray ] = setCellDepth( CellArray, NewDepth )
%SETCELLDEPTH - Adjusts the depth of the flat cell array
%  The depth can be increased unconditionally, it can be decreased if all
%  the top levels being discarded contain at max one cell array
%  (PartitionIndex of those levels can have at max 2 elements). If the
%  Depth is Zero, then it only initializes the depth. When Depth is
%  increased, the given array is pushed deeper. When it is decreased, the
%  deeper array is pulled to the top
	
	if NewDepth ~= floor(NewDepth)
		ME  = MException('FlatCellArray:InvalidInput', 'NewDepth must be an integer');
		throw(ME);
	end
	
	CurrentDepth = FlatCellArray.getCellDepth(CellArray);
	if CurrentDepth < NewDepth
		% Push the existing array deeper
		for i = CurrentDepth:NewDepth-1
			CellArray = {CellArray};
		end
	elseif CurrentDepth > NewDepth
		% Check if for all levels 1:CurrentDepth-NewDepth, the cell array
		% contains only one element
		IsValidReduction = true;
		CurrentLevel = CellArray;
		for i = 1:CurrentDepth-NewDepth
			if length(CurrentLevel) ~= 1
				IsValidReduction = false;
				break;
			end
			CurrentLevel = CurrentLevel{1};
		end
		if ~IsValidReduction
			ME = MException('FlatCellArr:InvelidDepthChange', ...
				'The depth of a FlatCellArray cannot be reduced if any of levels 1:OldDepth-NewDepth contain more than one cell');
			throw(ME);
		else
			% Push the existing deeper array to the top
			for i = 1:CurrentDepth-NewDepth
				CellArray = CellArray{1};
			end
		end
	end
end

