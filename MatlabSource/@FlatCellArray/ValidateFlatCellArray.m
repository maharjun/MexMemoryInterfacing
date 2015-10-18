function [isValid, Ex] = ValidateFlatCellArray(PartitionIndexIn, DataIn)
% ValidateFlatCellArray - Validates whether the given ParitionIndexIn and
% DataIn represent a valid FlatCellArray

% Validate The given PartitionIndex and Data.
isValid = true;
Ex = [];
if iscell(PartitionIndexIn)
	InitDepth = length(PartitionIndexIn);

	% validate if NonEmpty uint32 vectors
	for i = 1:InitDepth
		if ~isa(PartitionIndexIn{i}, 'uint32') || isempty(PartitionIndexIn{i})
			Ex = MException('FlatCellArray:InvalidInput','PartitionIndex{%d} expected to be a non-empty uint32 vector', i);
			isValid = false;
			return;
		end
	end

	% validate if sorted
	for i = 1:InitDepth
		if ~issorted(PartitionIndexIn{i})
			Ex = MException('FlatCellArray:InvalidInput','PartitionIndex{%d} expected to be sorted', i);
			isValid = false;
			return;
		end
	end

	% validate if First Elements are 0
	for i = 1:InitDepth
		if PartitionIndexIn{i}(1) ~= 0
			Ex = MException('FlatCellArray:InvalidInput','PartitionIndex{%d} expected to have first index as zero', i);
			isValid = false;
			return;
		end
	end

	% validate if BTE Elems are valid
	for i = 1:InitDepth
		if (i < InitDepth && PartitionIndexIn{i}(end) ~= length(PartitionIndexIn{i+1}) - 1) || ...
		   (i == InitDepth && PartitionIndexIn{i}(end) ~= length(DataIn))
			Ex = MException('FlatCellArray:InvalidInput','BTE Element of PartitionIndex{%d} is invalid', i);
			isValid = false;
			return;			
		end
	end
else
	Ex = MException('FlatCellArray:InvalidInput','PartitionIndex expected to be a cell array');
	isValid = false;
	return;
end

end