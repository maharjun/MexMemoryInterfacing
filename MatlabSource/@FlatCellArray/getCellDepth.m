function [ CellArrayDepth ] = getCellDepth( CellArray )
%GETCELLDEPTH This functions the depth of the cell array 
%   
%   [ CellArrayDepth ] = getCellDepth( CellArray )
%   
%   Input arguments:
%   
%     CellArray  - the cell array whose depth we wish to calculate
%   
%   Output arguments:
%   
%     CellArrayDepth = The depth of the cell array which givels the number
%                      of levels until one reaches a vector.
%     
%     e.g. 
%     
%     A cell array of vectors will have depth = 1
%     A cell array of cell arrays of vectors will have depth = 2
%     
%     Note that  the given  algorithm  will stop at  any depth at which all
%     elements are "not cells" note that this can include structs and vect-
%     ors or emptiness. The function does not perform any checks whatsoever
%     regarding  the class or  consistency of  the data stored  by the cell 
%     array.

	% validate if array is valid cell array
	if ~iscell(CellArray)
		CellArrayDepth = 0;
		return;
	end

	% find the indices of all subcells
	SubCellIndices = cellfun(@(x) iscell(x), CellArray);

	% get depth of subcells
	SubCellDepths = cellfun(@FlatCellArray.getCellDepth, CellArray(SubCellIndices));

	% return depth
	if ~isempty(SubCellDepths)
		MaxDepth = max(SubCellDepths);
	else
		MaxDepth = 0;
	end

	CellArrayDepth = 1 + MaxDepth;

end

