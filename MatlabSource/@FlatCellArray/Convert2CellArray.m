function CellArray = Convert2CellArray(obj)
% CONVERT2CELLARRAY This function converts the current Flat Cell Array to a Cell Array
%
%     CellArray = Convert2CellArray(this)
%   
%   This function converts the corresponding Flat Cell array into is
%   non-flat version an returns it. If the Flat Cell Array has properties
%   that do not conform to the requirements, then the behaviour is
%   undefined and may lead to exceptions being thrown
	
	% calculating Begin and End Arrays and calling Convert2CellArrayPartial
	FlatCellArrDepth = length(obj.PartitionIndex);
	BeginInds = zeros(FlatCellArrDepth, 1);
	EndInds   = cellfun(@length,obj.PartitionIndex) - 1;
	
	CellArray = obj.Convert2CellArrayPartial(1, BeginInds, EndInds);
end
