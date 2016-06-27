classdef FlatCellArray < hgsetget
	%FLATCELLARRAY implements a flattened version of the cell array
	%
	%   Detailed explanation goes here
	
	properties (SetAccess = private)
		% PartitionIndex - The cell array of the Partition Indices on the various levels
		% Indices which help partition the various levels of cell arrays 
		% and the final stored data into vectors.
		PartitionIndex = cell(0,1); 
		
		% Data - Data vector containing all the vectors in flattened form
		% This is the data which contains all the data in sequential
		% (flattened form). The partitioning of this vector into the
		% various vectors of the represented cell array is done using the
		% information stored in PartitionIndex.
		Data;
	end
	
	properties (Dependent)
		% Depth - The Depth of the Flattened Cell Array
		% This is the depth of the tree of the cell array represented by
		% the FlatCellArray. It is equal to the number of cells in 
		% PartitionIndex
		Depth;
	end
	
	% Proerty setget interfaces
	methods
		function Val = get.Depth(obj)
			Val = length(obj.PartitionIndex);
		end
		function set.Depth(obj, Val)
			% SET.DEPTH - Adjusts the depth of the flat cell array
			% The depth can be increased unconditionally, it can be
			% decreased if all the top levels being discarded contain at
			% max one cell array (PartitionIndex of those levels can have
			% at max 2 elements). If the Depth is Zero, then it only
			% initializes the depth. When Depth is increased, the given
			% array is pushed deeper. When it is decreased, the deeper
			% array is pulled to the top
			
			if obj.Depth == 0
				obj.PartitionIndex = cell(Val,1);
				for i = 1:Val
					obj.PartitionIndex{i} = zeros(1,1,'uint32');
				end
			elseif obj.Depth < Val
				% Push the existing array deeper
				NewPartitionIndex = cell(Val, 1);
				NewPartitionIndex(:) = {zeros(1,1, 'uint32')};
				newDepth = length(NewPartitionIndex); % possibly different from Val if Val is not an integer
				NewPartitionIndex(newDepth-obj.Depth+1:end) = obj.PartitionIndex;
				
				% if the current array is non-empty (length(PartitionIndex{1}) > 1)
				if length(obj.PartitionIndex{1}) > 1
					% Notify the one-above-old-top level of the size of the 
					% original top level.
					if newDepth-obj.Depth > 0
						NewPartitionIndex{newDepth-obj.Depth}(end+1) = length(obj.PartitionIndex{1});
					end

					% Notify all higher levels of the position of exactly one
					% element below them
					NewPartitionIndex(1:newDepth-obj.Depth-1) = {uint32([0; 1])};
				end
				
				obj.PartitionIndex = NewPartitionIndex;
			elseif obj.Depth > Val
				LevelSizes = cellfun(@length, obj.PartitionIndex);
				NewPartitionIndex = cell(Val, 1);
				newDepth = length(NewPartitionIndex);
				
				if any(LevelSizes(1:obj.Depth-newDepth) > 2)
					ME = MException('FlatCellArr:InvelidDepthChange', ...
						'The depth of a FlatCellArray cannot be reduced if any of levels 1:OldDepth-NewDepth contain more than one cell');
					throw(ME);
				else
					obj.PartitionIndex = obj.PartitionIndex(obj.Depth-newDepth+1:obj.Depth);
				end
			end
		end
	end
	
	% Private Methods List
	methods (Access = private)
		FlatCellArrOut = getSubFlatCellArr(FlatCellArrIn, Level, Indices )
		CellArray = Convert2CellArrayPartial(obj, DepthStartInd, BegInds, EndInds)
		BegInds = PushFlatCellArrayPartial(FlatCellArray, CellArray, DepthStartInd, BegInds)
	end
	
	% Static Methods List
	methods (Static)
		obj = FlattenCellArray(CellArray, InputArrayType, ActualDepth)
		CellArrayDepth = getCellDepth(CellArray)
		LevelSizeVect = getCellLevelSizes(CellArray)
		TypeString = getCellType(CellArray, CheckConsistency)
		CellArray = setCellDepth(CellArray, NewDepth)
		[isValid, Ex] = ValidateFlatCellArray(PartitionIndexIn, DataIn)
	end
	
	% Constructor List
	methods
		function obj = FlatCellArray(InitDepth, PartitionIndex, Data)
			% FlatCellArray - Initializes the cell array to null
			% 
			%   This Initializer initializes the member variables in the
			%   following manner
			%   
			%     obj.PartitionIndex = cell(0,1);
			%     obj.Data           = zeros(0,1);
			
			% null constructor
			if nargin == 0
				obj.PartitionIndex = cell(0,1);
				obj.Data           = zeros(0,1);
			% Empty constructor for particular depth.
			elseif nargin == 1
				obj.PartitionIndex = cell(0,1);
				obj.Data           = zeros(0,1);
				obj.Depth = InitDepth;
			elseif nargin == 2
				InputStruct = PartitionIndex;
				if isstruct(InputStruct) && isfield(InputStruct, 'ClassName') && strcmp(InputStruct.ClassName, 'FlatCellArray')
					% Validate Given Vectors
					[isValid, ME] = FlatCellArray.ValidateFlatCellArray(InputStruct.PartitionIndex, InputStruct.Data);

					if isValid
						% reshape to row vector
						for i = 1:InitDepth
							InputStruct.PartitionIndex{i} = reshape(InputStruct.PartitionIndex{i}, [length(InputStruct.PartitionIndex{i}) 1]);
						end

						obj.PartitionIndex = InputStruct.PartitionIndex;
						obj.Data           = InputStruct.Data;
					else
						throw(ME);
					end
				else
					ME = MException('FlatCellArray:InvalidInput', 'The 2nd argument should be a struct with "ClassName" Property = "FlatCellArray"');
					throw(ME);
				end
			elseif nargin == 3
				% Validate Given Vectors
				[isValid, ME] = FlatCellArray.ValidateFlatCellArray(PartitionIndex, Data);
				
				if isValid
					% reshape to row vector
					for i = 1:InitDepth
						PartitionIndex{i} = reshape(PartitionIndex{i}, [length(PartitionIndex{i}) 1]);
					end

					obj.PartitionIndex = PartitionIndex;
					obj.Data = Data;
				else
					throw(ME);
				end
			end
		end 
	end
	
	% Indexing Methods
	methods
		[varargout] = subsref(obj, Inds);
	end
	
	% Misc Methods
	methods (Access=public)
		CellArray = Convert2CellArray(obj)
		Pushback(obj, obj2Push, DepthStartInd);
		function obj = copy(this)
			obj = FlatCellArray();
			obj.PartitionIndex = this.PartitionIndex;
			obj.Data           = this.Data;
		end
		function structOut = Convert2Struct(obj)
			structOut.ClassName = 'FlatCellArray';
			structOut.PartitionIndex = obj.PartitionIndex;
			structOut.Data = obj.Data;
		end
	end
end


