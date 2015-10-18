function FlatCellArrOut = subsref(obj, Inds)
%SUBSREF Returns a FlatCellArray that represents the cell array indexed
%   Detailed explanation goes here
	
	% Handling the case of a . indexing at any point.
	
	DefaultInds = Inds([]);
	for i = 1:length(Inds)
		if strcmp(Inds(i).type, '.')
			DefaultInds = Inds(i:end);
			Inds = Inds(1:i-1);
			break;
		end
	end
	
	% In the event of purely struct indexing
	if isempty(Inds)
		FlatCellArrOut = builtin('subsref', obj, DefaultInds);
		return;
	end
	
	IndexingDepth = length(Inds);
	
	% Validate given indexing
	% ensure () indexing is only at the end
	for i = 1:IndexingDepth-1
		if strcmp(Inds(i).type, '()')
			ME = MException('FlatCellArray:IndexingInvalid', '(...) indexing must be the last of the indexing');
			throw(ME);
		end
	end
	
	% Validate subs of each of the cell indexing
	% Ensure that all '{}' cells have at max one element
	% Ensure that subs are one dimentional
	% ensure that all indexing is either '{}' or '()'
	for i = 1:IndexingDepth
		if length(Inds(i).subs) > 1
			ME = MException('FlatCellArray:IndexingInvalid', 'cell and vector indexing must be 1-D');
			throw(ME);
		end
		if strcmp(Inds(i).type, '{}')
			if ischar(Inds(i).subs{1})
				if isempty(obj.PartitionIndex) || isempty(obj.PartitionIndex{1})
					Inds(i).subs{1} = zeros(0,1);
				else
					Inds(i).subs{i} = 1;
				end
			else
				if length(Inds(i).subs{1}) > 1
					Inds(i).subs{1} = Inds(i).subs{1}(1);
				end
			end
		elseif ~strcmp(Inds(i).type, '()')
			ME = MException('FlatCellArray:IndexingInvalid', 'Only Cell and Vector Indexing allowed');
			throw(ME);
		end
	end
	
	% Terminate indexing at empty state
	for i = 1:IndexingDepth
		if strcmp(Inds(i).type, '{}')
			if isempty(Inds(i).subs{1})
				% Return NOTHING. Empty cell indexing is undefined
				return;
			end
		end
		% Empty '()' indexing can only come at the end due to above
		% validations
	end
	
	% Calculate level
	if strcmp(Inds(end).type, '()')
		Level = IndexingDepth;
	else
		Level = IndexingDepth + 1;
	end
	
	% Validate indexing depth
	if Level > obj.Depth + 1
		ME = MException('FlatCellArray:IndexingInvalid', '(...) Requested Depth (%d) must be <= Depth of FlatCellArray + 1 = %d', Level, obj.Depth+1);
		throw(ME);
	end
	
	% Calculate LevelIndices
	LevelIndices = 1:length(obj.PartitionIndex{1})-1;
	for i = 1:length(Inds)
		if strcmp(Inds(i).type, '{}')
			CurrentIndex = LevelIndices(Inds(i).subs{1});
			LevelIndices = obj.PartitionIndex{i}(CurrentIndex)+1:obj.PartitionIndex{i}(CurrentIndex+1);
		else
			LevelIndices = LevelIndices(Inds(i).subs{1});
		end
	end
	
	FlatCellArrOut = obj.getSubFlatCellArr(Level, LevelIndices);
	
	% Perform Default Indexing
	if ~isempty(DefaultInds)
		FlatCellArrOut = builtin('subsref', FlatCellArrOut, DefaultInds);
	end
end

